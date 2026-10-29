# Lift relocation-free production COFF function slices. Ghidra 10 / Jython.
# @category Kya.Verification
from java.nio.file import Files, Paths
from com.google.gson import JsonParser, JsonArray, GsonBuilder
from totala import PcodeExporter

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
    if function.getBody().getMinAddress().compareTo(address) < 0 or function.getBody().getMaxAddress().compareTo(end) > 0:
        raise RuntimeError('Function escaped COFF slice')
    exported = PcodeExporter.export(currentProgram, function, 4096)
    exported.add('coff', item)
    functions.add(exported)
manifest.add('exports', functions)
Files.writeString(Paths.get(args[1]), GsonBuilder().setPrettyPrinting().create().toJson(manifest))
