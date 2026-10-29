// Read-only export: ExportPcode.java <entry-address> <output.json>
// @category Kya.Verification
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import java.nio.file.*;
import com.google.gson.GsonBuilder;
import totala.PcodeExporter;

public class ExportPcode extends GhidraScript {
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length != 2) throw new IllegalArgumentException("Expected entry-address output.json");
        Function function = currentProgram.getFunctionManager().getFunctionAt(toAddr(args[0]));
        if (function == null) throw new IllegalArgumentException("No function at " + args[0]);
        Files.writeString(Path.of(args[1]), new GsonBuilder().setPrettyPrinting().create()
            .toJson(PcodeExporter.export(currentProgram, function, 65536)));
    }
}
