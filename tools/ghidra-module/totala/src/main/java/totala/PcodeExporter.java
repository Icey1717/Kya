package totala;

import com.google.gson.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.pcode.*;
import ghidra.framework.Application;
import ghidra.program.model.mem.MemoryAccessException;

/** Read-only raw instruction semantics. Delay-slot instructions are exported only
 * through their owning instruction, because Ghidra includes their p-code there. */
public final class PcodeExporter {
    private PcodeExporter() {}

    private static JsonObject identity(Instruction instruction) {
        JsonObject result = new JsonObject();
        result.addProperty("address", instruction.getAddress().toString());
        result.addProperty("mnemonic", instruction.getMnemonicString());
        result.addProperty("length", instruction.getLength());
        StringBuilder bytes = new StringBuilder();
        try {
            for (byte value : instruction.getBytes()) bytes.append(String.format("%02x", value & 255));
        } catch (MemoryAccessException error) {
            throw new IllegalArgumentException("Cannot read instruction bytes", error);
        }
        result.addProperty("bytes", bytes.toString());
        JsonArray operands = new JsonArray();
        for (int index = 0; index < instruction.getNumOperands(); index++)
            operands.add(instruction.getDefaultOperandRepresentation(index));
        result.add("operands", operands);
        return result;
    }

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
        result.addProperty("instruction_semantics_version", 2);
        result.addProperty("ghidra_version", Application.getApplicationVersion());
        result.addProperty("exporter_version", "kya-pcode-2");
        result.addProperty("program", program.getName());
        result.addProperty("executable_sha256", program.getExecutableSHA256());
        result.addProperty("language", program.getLanguageID().toString());
        result.addProperty("language_version", program.getLanguage().getVersion());
        result.addProperty("language_minor_version", program.getLanguage().getMinorVersion());
        result.addProperty("compiler", program.getCompilerSpec().getCompilerSpecID().toString());
        result.addProperty("big_endian", program.getLanguage().isBigEndian());
        result.addProperty("entry", function.getEntryPoint().toString());
        result.addProperty("name", function.getName(true));
        JsonArray instructions = new JsonArray();
        int count = 0;
        for (Instruction instruction : program.getListing().getInstructions(function.getBody(), true)) {
            if (instruction.isInDelaySlot()) continue;
            JsonObject item = identity(instruction);
            item.addProperty("address", instruction.getAddress().toString());
            item.addProperty("text", instruction.toString());
            item.addProperty("delay_slots", instruction.getDelaySlotDepth());
            item.addProperty("flow", instruction.getFlowType().toString());
            JsonArray slots = new JsonArray();
            Instruction slot = instruction;
            for (int index = 0; index < instruction.getDelaySlotDepth(); index++) {
                slot = slot.getNext();
                if (slot == null || !slot.isInDelaySlot())
                    throw new IllegalArgumentException("Missing delay-slot provenance");
                slots.add(identity(slot));
            }
            item.add("delay_slot_instructions", slots);
            JsonArray operations = new JsonArray();
            for (PcodeOp op : instruction.getPcode()) {
                if (++count > limit) throw new IllegalArgumentException("P-code limit exceeded; no partial export returned");
                JsonObject operation = new JsonObject();
                operation.addProperty("opcode", op.getMnemonic());
                operation.addProperty("sequence_address", op.getSeqnum().getTarget().toString());
                operation.addProperty("sequence_index", op.getSeqnum().getTime());
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
