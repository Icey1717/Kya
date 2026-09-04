/*
 * PatchMipsRelocatedJumps.java
 *
 * Applies MIPS R_MIPS_26 relocations which Ghidra imported but did not apply
 * to relocatable PS2 IOP modules.  This fixes direct j/jal operands and their
 * references without changing the MCP.
 *
 * Run from Ghidra's Script Manager with the affected program open.
 */

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.reloc.Relocation;
import ghidra.program.model.reloc.RelocationTable;
import ghidra.program.model.symbol.RefType;
import ghidra.program.model.symbol.ReferenceManager;
import ghidra.program.model.symbol.SourceType;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;

import java.util.Iterator;

public class PatchMipsRelocatedJumps extends GhidraScript {

	private static final int R_MIPS_26 = 4;
	private static final int MIPS_J = 2;
	private static final int MIPS_JAL = 3;

	@Override
	public void run() throws Exception {
		RelocationTable relocations = currentProgram.getRelocationTable();
		if (!relocations.isRelocatable()) {
			println("The active program has no relocatable ELF relocation table.");
			return;
		}

		Memory memory = currentProgram.getMemory();
		MemoryBlock text = memory.getBlock(".text");
		if (text == null) {
			println("Could not find the .text memory block.");
			return;
		}

		println("Patching R_MIPS_26 relocations in " + currentProgram.getName());
		println(".text mapped at " + text.getStart());

		int transaction = currentProgram.startTransaction("Patch MIPS relocated jumps");
		boolean commit = false;
		int seen = 0;
		int patched = 0;
		int skipped = 0;

		try {
			Iterator<Relocation> iterator = relocations.getRelocations();
			while (iterator.hasNext()) {
				Relocation relocation = iterator.next();
				if (relocation.getType() != R_MIPS_26)
					continue;

				seen++;
				Address site = relocation.getAddress();
				int instruction = memory.getInt(site);
				int opcode = instruction >>> 26;
				if (opcode != MIPS_J && opcode != MIPS_JAL) {
					println("SKIP " + site + ": relocation is not j/jal (opcode " + opcode + ")");
					skipped++;
					continue;
				}

				long rawTarget = (instruction & 0x03ffffffL) << 2;
				Address target = findMappedSymbol(relocation.getSymbolName());
				if (target == null)
					target = text.getStart().add(rawTarget);

				long pcRegion = (site.getOffset() + 4) & 0xf0000000L;
				long targetRegion = target.getOffset() & 0xf0000000L;
				if (pcRegion != targetRegion) {
					println("SKIP " + site + ": target " + target + " is outside the MIPS J region");
					skipped++;
					continue;
				}

				int patchedInstruction = (instruction & 0xfc000000) |
					(int)((target.getOffset() >>> 2) & 0x03ffffffL);
				memory.setInt(site, patchedInstruction);

				ReferenceManager references = currentProgram.getReferenceManager();
				references.removeAllReferencesFrom(site);
				RefType referenceType = opcode == MIPS_JAL
					? RefType.UNCONDITIONAL_CALL
					: RefType.UNCONDITIONAL_JUMP;
				references.addMemoryReference(site, target, referenceType, SourceType.ANALYSIS, 0);

				println(String.format("PATCH %s: %s -> %s (%s)", site,
					formatAddress(rawTarget), target, relocation.getSymbolName(),
					opcode == MIPS_JAL ? "jal" : "j"));
				patched++;
			}

			commit = true;
		}
		finally {
			currentProgram.endTransaction(transaction, commit);
		}

		println("R_MIPS_26 relocations: " + seen + ", patched: " + patched + ", skipped: " + skipped);
		println("Save the program if the patched analysis is correct.");
	}

	private Address findMappedSymbol(String symbolName) {
		if (symbolName == null || symbolName.isEmpty())
			return null;

		SymbolIterator symbols = currentProgram.getSymbolTable().getSymbols(symbolName);
		while (symbols.hasNext()) {
			Symbol symbol = symbols.next();
			Address address = symbol.getAddress();
			if (currentProgram.getMemory().getBlock(address) != null)
				return address;
		}
		return null;
	}

	private String formatAddress(long address) {
		return String.format("0x%08x", address);
	}
}
