// Export requested function decompilations for an offline source audit. Read-only.
// @category Kya.Audits

import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.google.gson.JsonElement;
import com.google.gson.JsonParser;
import java.io.BufferedWriter;
import java.io.File;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.util.HashSet;
import java.util.Set;

public class ExportFunctionAudit extends GhidraScript {
    @Override
    public void run() throws Exception {
        if (currentProgram == null) throw new IllegalStateException("Open the PS2 executable first");
        String[] args = getScriptArgs();
        File inventory = args.length == 2 ? new File(args[0]) : askFile("Select audit inventory.json", "Open");
        File output = args.length == 2 ? new File(args[1]) : askFile("Save PS2 audit snapshot (.jsonl)", "Save");
        JsonObject manifest = JsonParser.parseString(Files.readString(inventory.toPath(), StandardCharsets.UTF_8)).getAsJsonObject();
        Set<String> names = new HashSet<>();
        Set<String> addresses = new HashSet<>();
        for (JsonElement element : manifest.getAsJsonArray("functions")) {
            String name = element.getAsJsonObject().get("name").getAsString();
            names.add(name);
            if (name.matches(".*(?:^|::)FUN_[0-9a-fA-F]{8}$")) addresses.add(name.substring(name.length() - 8).toLowerCase());
        }
        JsonObject identity = new JsonObject();
        identity.addProperty("program", currentProgram.getName());
        identity.addProperty("executable_path", currentProgram.getExecutablePath());
        identity.addProperty("image_base", currentProgram.getImageBase().toString());
        identity.addProperty("language", currentProgram.getLanguageID().toString());
        identity.addProperty("executable_sha256", currentProgram.getExecutableSHA256());
        JsonObject header = new JsonObject();
        header.addProperty("schema", 1);
        header.add("program", identity);
        Gson gson = new Gson();
        File temporary = new File(output.getPath() + ".tmp");
        int exported = 0;
        DecompInterface decompiler = new DecompInterface();
        try {
            if (!decompiler.openProgram(currentProgram)) throw new IllegalStateException("Cannot open decompiler");
            try (BufferedWriter writer = Files.newBufferedWriter(temporary.toPath(), StandardCharsets.UTF_8)) {
                writer.write(gson.toJson(header));
                writer.newLine();
                FunctionIterator functions = currentProgram.getFunctionManager().getFunctions(true);
                while (functions.hasNext()) {
                    monitor.checkCancelled();
                    Function function = functions.next();
                    String owner = function.getParentNamespace().getName(true);
                    String qualified = owner.equals("Global") || function.getName().contains("::")
                        ? function.getName() : owner + "::" + function.getName();
                    String address = function.getEntryPoint().toString();
                    if (!names.contains(qualified) && !addresses.contains(address.toLowerCase())) continue;
                    monitor.setMessage("Exporting " + qualified + " at " + address);
                    JsonObject metadata = new JsonObject();
                    metadata.addProperty("entry", address);
                    metadata.addProperty("name", function.getName());
                    metadata.addProperty("namespace", owner);
                    metadata.addProperty("signature", function.getSignature().toString());
                    JsonObject entry = new JsonObject();
                    entry.add("function", metadata);
                    DecompileResults result = decompiler.decompileFunction(function, 60, monitor);
                    if (result.decompileCompleted() && result.getDecompiledFunction() != null) {
                        entry.addProperty("c", result.getDecompiledFunction().getC());
                    } else {
                        entry.addProperty("error", result.getErrorMessage());
                    }
                    writer.write(gson.toJson(entry));
                    writer.newLine();
                    writer.flush();
                    exported++;
                }
            }
            monitor.checkCancelled();
            Files.move(temporary.toPath(), output.toPath(), StandardCopyOption.REPLACE_EXISTING);
        } finally {
            decompiler.dispose();
        }
        println("Exported " + exported + " functions to " + output + ". Program was not modified.");
    }
}
