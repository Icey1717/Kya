/* ###
 * IP: GHIDRA
 * Licensed under the Apache License, Version 2.0.
 */
package totala;

import java.io.IOException;

import ghidra.app.plugin.PluginCategoryNames;
import ghidra.app.plugin.ProgramPlugin;
import ghidra.framework.options.OptionsChangeListener;
import ghidra.framework.options.ToolOptions;
import ghidra.framework.plugintool.PluginInfo;
import ghidra.framework.plugintool.PluginTool;
import ghidra.framework.plugintool.util.PluginStatus;
import ghidra.util.Msg;

/** Lifecycle owner for the localhost-only, persistent Ghidra MCP bridge. */
//@formatter:off
@PluginInfo(
	status = PluginStatus.STABLE,
	packageName = "Totala",
	category = PluginCategoryNames.COMMON,
	shortDescription = "Persistent localhost Ghidra MCP bridge",
	description = "Serves the Totala JSON-lines protocol on localhost and commits and saves every mutation independently."
)
//@formatter:on
public final class totalaPlugin extends ProgramPlugin implements OptionsChangeListener {
	private static final String OPTIONS = "Totala MCP Bridge";
	private static final String PORT = "Port";
	private static final String LABEL = "Instance label";
	private static final int DEFAULT_PORT = 18437;
	private static final String DEFAULT_LABEL = "totala";

	private ToolOptions options;
	private GhidraMcpBridge bridge;

	public totalaPlugin(PluginTool tool) {
		super(tool, false, false);
	}

	@Override
	protected void init() {
		super.init();
		options = tool.getOptions(OPTIONS);
		options.registerOption(PORT, DEFAULT_PORT, null,
			"TCP port on 127.0.0.1. Use a distinct port for each Ghidra instance.");
		options.registerOption(LABEL, DEFAULT_LABEL, null,
			"Stable instance label returned by the status command.");
		options.addOptionsChangeListener(this);
		restartBridge();
	}

	@Override
	public void optionsChanged(ToolOptions changedOptions, String optionName, Object oldValue,
			Object newValue) {
		if (PORT.equals(optionName) || LABEL.equals(optionName)) {
			restartBridge();
		}
	}

	private synchronized void restartBridge() {
		stopBridge();
		int port = options.getInt(PORT, DEFAULT_PORT);
		String label = options.getString(LABEL, DEFAULT_LABEL).trim();
		if (label.isEmpty()) {
			label = DEFAULT_LABEL;
		}
		try {
			bridge = new GhidraMcpBridge(() -> currentProgram, port, label);
			bridge.start();
			Msg.info(this, "Totala MCP bridge '" + label + "' listening on 127.0.0.1:" + port);
		}
		catch (IllegalArgumentException | IOException error) {
			bridge = null;
			Msg.showError(this, null, "Totala MCP Bridge",
				"Could not start the localhost bridge: " + error.getMessage());
		}
	}

	private synchronized void stopBridge() {
		if (bridge != null) {
			bridge.close();
			bridge = null;
		}
	}

	@Override
	protected void dispose() {
		if (options != null) {
			options.removeOptionsChangeListener(this);
		}
		stopBridge();
		super.dispose();
	}
}
