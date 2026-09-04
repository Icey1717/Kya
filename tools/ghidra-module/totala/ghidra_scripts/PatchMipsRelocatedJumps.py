# PatchMipsRelocatedJumps.py
#@category Kya
#
# Applies MIPS R_MIPS_26 relocations which Ghidra imported but did not apply
# to relocatable PS2 IOP modules. This fixes direct j/jal operands and their
# references without changing the MCP.

from ghidra.program.model.symbol import RefType, SourceType

R_MIPS_26 = 4
MIPS_J = 2
MIPS_JAL = 3


def find_mapped_symbol(symbol_name):
    if symbol_name is None or symbol_name == "":
        return None

    symbols = currentProgram.getSymbolTable().getSymbols(symbol_name)
    while symbols.hasNext():
        symbol = symbols.next()
        address = symbol.getAddress()
        if currentProgram.getMemory().getBlock(address) is not None:
            return address
    return None


def format_address(address):
    return "0x%08x" % address


relocations = currentProgram.getRelocationTable()
if not relocations.isRelocatable():
    println("The active program has no relocatable ELF relocation table.")
else:
    memory = currentProgram.getMemory()
    text = memory.getBlock(".text")
    if text is None:
        println("Could not find the .text memory block.")
    else:
        println("Patching R_MIPS_26 relocations in " + currentProgram.getName())
        println(".text mapped at " + str(text.getStart()))

        transaction = currentProgram.startTransaction("Patch MIPS relocated jumps")
        commit = False
        seen = 0
        patched = 0
        skipped = 0

        try:
            iterator = relocations.getRelocations()
            while iterator.hasNext():
                relocation = iterator.next()
                if relocation.getType() != R_MIPS_26:
                    continue

                seen += 1
                site = relocation.getAddress()
                instruction = memory.getInt(site)
                opcode = (instruction >> 26) & 0x3f
                if opcode != MIPS_J and opcode != MIPS_JAL:
                    println("SKIP %s: relocation is not j/jal (opcode %d)" %
                            (site, opcode))
                    skipped += 1
                    continue

                raw_target = (instruction & 0x03ffffff) << 2
                target = find_mapped_symbol(relocation.getSymbolName())
                if target is None:
                    target = text.getStart().add(raw_target)

                pc_region = (site.getOffset() + 4) & 0xf0000000
                target_region = target.getOffset() & 0xf0000000
                if pc_region != target_region:
                    println("SKIP %s: target %s is outside the MIPS J region" %
                            (site, target))
                    skipped += 1
                    continue

                patched_instruction = (instruction & 0xfc000000) | \
                    ((target.getOffset() >> 2) & 0x03ffffff)
                memory.setInt(site, patched_instruction)

                references = currentProgram.getReferenceManager()
                references.removeAllReferencesFrom(site)
                reference_type = (RefType.UNCONDITIONAL_CALL if opcode == MIPS_JAL
                                  else RefType.UNCONDITIONAL_JUMP)
                references.addMemoryReference(site, target, reference_type,
                                              SourceType.ANALYSIS, 0)

                mnemonic = "jal" if opcode == MIPS_JAL else "j"
                println("PATCH %s: %s -> %s (%s, %s)" %
                        (site, format_address(raw_target), target,
                         relocation.getSymbolName(), mnemonic))
                patched += 1

            commit = True
        finally:
            currentProgram.endTransaction(transaction, commit)

        println("R_MIPS_26 relocations: %d, patched: %d, skipped: %d" %
                (seen, patched, skipped))
        println("Save the program if the patched analysis is correct.")
