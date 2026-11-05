# Lift relocation-free production COFF function slices. Ghidra 10 / Jython.
# @category Kya.Verification
from java.nio.file import Files, Paths
from com.google.gson import JsonParser, JsonArray, GsonBuilder
from totala import PcodeExporter
from ghidra.program.model.address import AddressSet

args = getScriptArgs()
manifest = JsonParser.parseString(Files.readString(Paths.get(args[0]))).getAsJsonObject()
functions = JsonArray()
for value in manifest.getAsJsonArray('functions'):
    item = value.getAsJsonObject()
    address = toAddr(item.get('blob_offset').getAsLong())
    if not disassemble(address):
        raise RuntimeError('Disassembly failed at ' + str(address))
    function = createFunction(address, item.get('name').getAsString())
    if function is None:
        raise RuntimeError('Function creation failed at ' + str(address))
    end = address.add(item.get('size').getAsLong() - 1)
    if item.has('jump_tables'):
        for table in item.getAsJsonArray('jump_tables'):
            for target in table.getAsJsonObject().getAsJsonArray('targets'):
                destination = toAddr(target.getAsLong())
                if destination.compareTo(address) < 0 or destination.compareTo(end) > 0:
                    raise RuntimeError('Jump table target leaves COFF slice')
                if not disassemble(destination) or currentProgram.getListing().getInstructionAt(destination) is None:
                    raise RuntimeError('Jump table target is not an instruction boundary')
        # Entry flow alone stops at computed switches. Include the decoded
        # case blocks reached from every checked COFF relocation target.
        body = AddressSet()
        instructions = currentProgram.getListing().getInstructions(address, True)
        while instructions.hasNext():
            instruction = instructions.next()
            if instruction.getAddress().compareTo(end) > 0:
                break
            if instruction.getMaxAddress().compareTo(end) > 0:
                raise RuntimeError('Instruction crosses COFF function boundary')
            body.addRange(instruction.getAddress(), instruction.getMaxAddress())
        function.setBody(body)
    if function.getBody().getMinAddress().compareTo(address) < 0 or function.getBody().getMaxAddress().compareTo(end) > 0:
        raise RuntimeError('Function escaped COFF slice')
    exported = PcodeExporter.export(currentProgram, function, 4096)
    exported.add('coff', item)
    functions.add(exported)
manifest.add('exports', functions)
Files.writeString(Paths.get(args[1]), GsonBuilder().setPrettyPrinting().create().toJson(manifest))
