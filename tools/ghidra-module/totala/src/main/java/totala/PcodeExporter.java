package totala;

import com.google.gson.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.pcode.*;

/** Read-only raw instruction semantics. Delay-slot instructions are exported only
 * through their owning instruction, because Ghidra includes their p-code there. */
public final class PcodeExporter {
    private PcodeExporter() {}

    private static JsonElement node(Varnode value) {
        if (value == null) return JsonNull.INSTANCE;
        JsonObject result = new JsonObject();
        result.addProperty("space", value.getAddress().getAddressSpace().getName());
        result.addProperty("offset", Long.toUnsignedString(value.getOffset(), 16));
        result.addProperty("size", value.getSize());
        result.addProperty("constant", value.isConstant());
        return result;
    }

    public static JsonObject export(Program program, Function function, int limit) {
        if (limit < 1 || limit > 65536) throw new IllegalArgumentException("Invalid operation limit");
        JsonObject result = new JsonObject();
        result.addProperty("schema", "kya.raw-pcode.v1");
        result.addProperty("program", program.getName());
        result.addProperty("executable_sha256", program.getExecutableSHA256());
        result.addProperty("language", program.getLanguageID().toString());
        result.addProperty("compiler", program.getCompilerSpec().getCompilerSpecID().toString());
        result.addProperty("big_endian", program.getLanguage().isBigEndian());
        result.addProperty("entry", function.getEntryPoint().toString());
        result.addProperty("name", function.getName(true));
        JsonArray instructions = new JsonArray();
        int count = 0;
        for (Instruction instruction : program.getListing().getInstructions(function.getBody(), true)) {
            if (instruction.isInDelaySlot()) continue;
            JsonObject item = new JsonObject();
            item.addProperty("address", instruction.getAddress().toString());
            item.addProperty("text", instruction.toString());
            item.addProperty("delay_slots", instruction.getDelaySlotDepth());
            item.addProperty("flow", instruction.getFlowType().toString());
            JsonArray operations = new JsonArray();
            for (PcodeOp op : instruction.getPcode()) {
                if (++count > limit) throw new IllegalArgumentException("P-code limit exceeded; no partial export returned");
                JsonObject operation = new JsonObject();
                operation.addProperty("opcode", op.getMnemonic());
                operation.add("output", node(op.getOutput()));
                JsonArray inputs = new JsonArray();
                for (Varnode input : op.getInputs()) inputs.add(node(input));
                operation.add("inputs", inputs);
                operations.add(operation);
            }
            item.add("ops", operations);
            instructions.add(item);
        }
        result.add("instructions", instructions);
        result.addProperty("operation_count", count);
        result.addProperty("complete", true);
        return result;
    }
}
