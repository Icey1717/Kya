// Export all named virtual tables without changing the selected program.
// @category Kya.Audits

import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import java.io.File;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.Map;
import java.util.TreeMap;

public class ExportVtableMap extends GhidraScript {
    @Override
    public void run() throws Exception {
        if (currentProgram == null) {
            throw new IllegalStateException("Open the PS2 program first");
        }
        String[] args = getScriptArgs();
        File output = args.length == 1 ? new File(args[0]) : askFile("Export vtable map", "Save");
        Map<String, JsonArray> tables = new TreeMap<>();
        SymbolIterator symbols = currentProgram.getSymbolTable().getAllSymbols(true);
        while (symbols.hasNext()) {
            monitor.checkCancelled();
            Symbol symbol = symbols.next();
            String name = symbol.getName();
            if (!name.equals("_vt") && !name.equals("vtable") && !name.equals("vftable")) {
                continue;
            }
            String owner = symbol.getParentNamespace().getName(true);
            tables.computeIfAbsent(owner, key -> new JsonArray()).add(symbol.getAddress().toString());
        }
        JsonObject identity = new JsonObject();
        identity.addProperty("program", currentProgram.getName());
        identity.addProperty("image_base", currentProgram.getImageBase().toString());
        identity.addProperty("language", currentProgram.getLanguageID().toString());
        JsonObject result = new JsonObject();
        result.addProperty("schema", 1);
        result.add("program", identity);
        JsonObject exported = new JsonObject();
        tables.forEach(exported::add);
        result.add("tables", exported);
        Files.writeString(output.toPath(), new GsonBuilder().setPrettyPrinting().create().toJson(result), StandardCharsets.UTF_8);
        println("Exported " + tables.size() + " table owners to " + output);
    }
}
