/* ###
 * IP: GHIDRA
 * Licensed under the Apache License, Version 2.0.
 */
package totala;

import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.Set;
import java.util.concurrent.*;
import java.util.function.Supplier;

import com.google.gson.*;

import ghidra.program.model.listing.Program;
import ghidra.util.Msg;

/** A bounded JSON-lines server. All Ghidra access is serialized on one worker. */
final class GhidraMcpBridge implements AutoCloseable {
	static final int MAX_REQUEST_BYTES = 1024 * 1024;
	private static final int ACCEPT_BACKLOG = 16;
	private static final int SOCKET_TIMEOUT_MS = 70_000;

	private final Supplier<Program> programSupplier;
	private final int port;
	private final String label;
	private final Gson gson = new Gson();
	private final ThreadPoolExecutor requests;
	private final Set<Socket> clients = ConcurrentHashMap.newKeySet();
	private volatile boolean running;
	private ServerSocket server;
	private Thread acceptThread;

	GhidraMcpBridge(Supplier<Program> programSupplier, int port, String label) {
		if (port < 1 || port > 65535) {
			throw new IllegalArgumentException("Port must be between 1 and 65535");
		}
		this.programSupplier = programSupplier;
		this.port = port;
		this.label = label;
		requests = new ThreadPoolExecutor(1, 1, 0, TimeUnit.MILLISECONDS,
			new ArrayBlockingQueue<>(ACCEPT_BACKLOG), runnable -> {
				Thread thread = new Thread(runnable, "Totala-MCP-request");
				thread.setDaemon(true);
				return thread;
			}, new ThreadPoolExecutor.AbortPolicy());
	}

	void start() throws IOException {
		server = new ServerSocket();
		server.setReuseAddress(true);
		// Deliberately use the literal IPv4 loopback address, never a wildcard/interface address.
		server.bind(new InetSocketAddress(InetAddress.getByName("127.0.0.1"), port),
			ACCEPT_BACKLOG);
		running = true;
		acceptThread = new Thread(this::acceptLoop, "Totala-MCP-accept");
		acceptThread.setDaemon(true);
		acceptThread.start();
	}

	private void acceptLoop() {
		while (running) {
			try {
				Socket client = server.accept();
				client.setSoTimeout(SOCKET_TIMEOUT_MS);
				clients.add(client);
				try {
					requests.execute(() -> serve(client));
				}
				catch (RejectedExecutionException busy) {
					writeAndClose(client, error("busy", "Bridge request queue is full"));
				}
			}
			catch (SocketException closed) {
				if (running) {
					Msg.error(this, "Totala MCP accept failed", closed);
				}
			}
			catch (IOException error) {
				if (running) {
					Msg.error(this, "Totala MCP accept failed", error);
				}
			}
		}
	}

	private void serve(Socket client) {
		try (InputStream input = client.getInputStream();
				Writer output = new BufferedWriter(
					new OutputStreamWriter(client.getOutputStream(), StandardCharsets.UTF_8))) {
			JsonObject response;
			try {
				String line = readLine(input, MAX_REQUEST_BYTES);
				JsonObject request = JsonParser.parseString(line).getAsJsonObject();
				JsonElement result =
					new GhidraMcpCommandHandler(programSupplier, label).handle(request);
				response = success(result);
			}
			catch (BridgeException requestError) {
				response = error(requestError.code(), requestError.getMessage());
			}
			catch (JsonParseException | IllegalStateException requestError) {
				response = error("invalid_request", "Request must be one JSON object");
			}
			catch (SocketTimeoutException requestError) {
				response = error("timeout", "Socket request timed out");
			}
			catch (Exception requestError) {
				Msg.error(this, "Totala MCP request failed", requestError);
				response = error("internal_error", "Ghidra bridge request failed");
			}
			write(output, response);
		}
		catch (IOException error) {
			// The peer may have disconnected before receiving its response.
		}
		finally {
			clients.remove(client);
			try {
				client.close();
			}
			catch (IOException ignored) {
				// The peer may already have gone away.
			}
		}
	}

	static String readLine(InputStream input, int maximum) throws IOException, BridgeException {
		ByteArrayOutputStream bytes = new ByteArrayOutputStream();
		for (;;) {
			int value = input.read();
			if (value == -1) {
				if (bytes.size() == 0) {
					throw new EOFException("Client closed without a request");
				}
				break;
			}
			if (value == '\n') {
				break;
			}
			if (bytes.size() >= maximum) {
				throw new BridgeException("request_too_large",
					"Request exceeds " + maximum + " bytes");
			}
			if (value != '\r') {
				bytes.write(value);
			}
		}
		return bytes.toString(StandardCharsets.UTF_8);
	}

	private void writeAndClose(Socket client, JsonObject response) {
		try (Writer output = new BufferedWriter(
				new OutputStreamWriter(client.getOutputStream(), StandardCharsets.UTF_8))) {
			write(output, response);
		}
		catch (IOException ignored) {
			// The peer may already have gone away.
		}
		finally {
			clients.remove(client);
			try {
				client.close();
			}
			catch (IOException ignored) {
				// Best-effort close.
			}
		}
	}

	private void write(Writer output, JsonObject response) throws IOException {
		gson.toJson(response, output);
		output.write('\n');
		output.flush();
	}

	private static JsonObject success(JsonElement result) {
		JsonObject response = new JsonObject();
		response.addProperty("ok", true);
		response.add("result", result);
		return response;
	}

	private static JsonObject error(String code, String message) {
		JsonObject response = new JsonObject();
		response.addProperty("ok", false);
		response.addProperty("code", code);
		response.addProperty("error", message == null ? code : message);
		return response;
	}

	@Override
	public void close() {
		running = false;
		if (server != null) {
			try {
				server.close();
			}
			catch (IOException ignored) {
				// Closing is best-effort.
			}
		}
		requests.shutdownNow();
		for (Socket client : clients) {
			try {
				client.close();
			}
			catch (IOException ignored) {
				// Best-effort close.
			}
		}
		clients.clear();
		try {
			requests.awaitTermination(2, TimeUnit.SECONDS);
		}
		catch (InterruptedException interrupted) {
			Thread.currentThread().interrupt();
		}
		if (acceptThread != null) {
			try {
				acceptThread.join(2_000);
			}
			catch (InterruptedException interrupted) {
				Thread.currentThread().interrupt();
			}
		}
	}
}

final class BridgeException extends Exception {
	private final String code;

	BridgeException(String code, String message) {
		super(message);
		this.code = code;
	}

	String code() {
		return code;
	}
}
