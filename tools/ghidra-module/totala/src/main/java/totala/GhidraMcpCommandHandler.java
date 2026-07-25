/* ###
 * IP: GHIDRA
 * Licensed under the Apache License, Version 2.0.
 */
package totala;

import java.util.*;
import java.util.function.Supplier;

import com.google.gson.*;

import ghidra.app.decompiler.*;
import ghidra.framework.model.DomainFile;
import ghidra.program.model.address.*;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import ghidra.program.model.symbol.*;
import ghidra.util.task.TaskMonitor;

/** Protocol implementation. The enclosing server serializes every call to this class. */
final class GhidraMcpCommandHandler {
	private static final Object CONSUMER = GhidraMcpCommandHandler.class;

	private final Supplier<Program> programSupplier;
	private final String label;

	GhidraMcpCommandHandler(Supplier<Program> programSupplier, String label) {
		this.programSupplier = programSupplier;
		this.label = label;
	}

	JsonElement handle(JsonObject request) throws BridgeException {
		String command = requiredString(request, "command");
		Program program = programSupplier.get();
		if (program == null) {
			throw error("no_program", "No program is open in this Ghidra tool");
		}
		if (!program.addConsumer(CONSUMER)) {
			throw error("no_program", "The active program closed before the request began");
		}
		try {
			return switch (command) {
				case "status" -> status(program);
				case "list_functions" -> listFunctions(program, request);
				case "get_function" -> functionData(findFunction(program, request));
				case "decompile" -> decompile(program, request);
				case "references_to" -> referencesTo(program, request);
				case "inspect_data" -> inspectData(program, request);
				case "disassemble" -> disassemble(program, request);
				case "references_from" -> referencesFrom(program, request);
				case "decompile_context" -> decompile(program, request);
				case "search_strings" -> searchStrings(program, request);
				case "get_function_variables" -> functionVariables(findFunction(program, request));
				case "get_data_type_members" -> dataTypeMembers(resolveComposite(program, request));
				case "create_data_type" -> mutate(program, request, command, () ->
					createDataType(program, request));
				case "get_labels" -> getLabels(program, request);
				case "rename_function" -> mutate(program, request, command, () -> {
					Function function = findFunction(program, request);
					function.setName(requiredString(request, "new_name"), SourceType.USER_DEFINED);
					return functionData(function);
				});
				case "set_comment" -> mutate(program, request, command, () -> {
					Function function = findFunction(program, request);
					function.setComment(requiredString(request, "comment"));
					return functionData(function);
				});
				case "rename_function_parameter" -> mutate(program, request, command, () -> {
					Function function = findFunction(program, request);
					Parameter parameter = selectParameter(function, request);
					parameter.setName(requiredString(request, "new_name"), SourceType.USER_DEFINED);
					return functionVariables(function);
				});
				case "rename_local_variable" -> mutate(program, request, command, () -> {
					Function function = findFunction(program, request);
					Variable local = selectLocal(function, request);
					local.setName(requiredString(request, "new_name"), SourceType.USER_DEFINED);
					return functionVariables(function);
				});
				case "set_function_variable_type" -> mutate(program, request, command, () -> {
					Function function = findFunction(program, request);
					selectVariable(function, request).setDataType(resolveMemberDataType(program, requiredString(request, "data_type")), SourceType.USER_DEFINED);
					return functionVariables(function);
				});
				case "set_function_variable_comment" -> mutate(program, request, command, () -> {
					Function function = findFunction(program, request);
					selectVariable(function, request).setComment(requiredString(request, "comment"));
					return functionVariables(function);
				});
				case "rename_data_type_member" -> mutate(program, request, command, () -> {
					Composite composite = resolveComposite(program, request);
					selectComponent(composite, request, "old_name").setFieldName(
						requiredString(request, "new_name"));
					return dataTypeMembers(composite);
				});
				case "set_data_type_member_comment" -> mutate(program, request, command, () -> {
					Composite composite = resolveComposite(program, request);
					selectComponent(composite, request, "field_name").setComment(
						requiredString(request, "comment"));
					return dataTypeMembers(composite);
				});
				case "set_data_type_member_type" -> mutate(program, request, command, () ->
					setMemberType(program, request));
				case "add_data_type_member" -> mutate(program, request, command, () ->
					addMember(program, request));
				case "set_label" -> mutate(program, request, command, () -> setLabel(program, request));
				case "remove_label" -> mutate(program, request, command, () ->
					removeLabel(program, request));
				default -> throw error("unknown_command", "Unknown command: " + command);
			};
		}
		catch (BridgeException bridgeError) {
			throw bridgeError;
		}
		catch (Exception requestError) {
			String message = requestError.getMessage();
			throw error("request_failed",
				message == null || message.isBlank() ? requestError.getClass().getSimpleName() : message);
		}
		finally {
			program.release(CONSUMER);
		}
	}

	private JsonObject status(Program program) {
		JsonObject result = new JsonObject();
		result.addProperty("instance", label);
		result.addProperty("program", program.getName());
		result.addProperty("executable_path", program.getExecutablePath());
		result.addProperty("language", program.getLanguageID().toString());
		result.addProperty("image_base", program.getImageBase().toString());
		DomainFile file = program.getDomainFile();
		result.addProperty("project_file", file == null ? null : file.getPathname());
		result.addProperty("writable", file != null && file.isInWritableProject() &&
			!file.isReadOnly() && program.canSave());
		return result;
	}

	private JsonObject listFunctions(Program program, JsonObject request) {
		int limit = boundedInt(request, "limit", 100, 1, 1000);
		String filter = optionalString(request, "filter", "").toLowerCase(Locale.ROOT);
		JsonArray functions = new JsonArray();
		FunctionIterator iterator = program.getFunctionManager().getFunctions(true);
		while (iterator.hasNext() && functions.size() < limit) {
			Function function = iterator.next();
			if (function.getName().toLowerCase(Locale.ROOT).contains(filter)) {
				functions.add(functionData(function));
			}
		}
		JsonObject result = new JsonObject();
		result.add("functions", functions);
		result.addProperty("limit", limit);
		return result;
	}

	private JsonObject functionData(Function function) {
		JsonObject result = new JsonObject();
		result.addProperty("name", function.getName());
		result.addProperty("entry", function.getEntryPoint().toString());
		result.addProperty("body", function.getBody().toString());
		result.addProperty("signature", function.getSignature().toString());
		result.addProperty("namespace", function.getParentNamespace().getName(true));
		String comment = function.getComment();
		if (comment != null) {
			result.addProperty("comment", comment);
		}
		return result;
	}

	private JsonObject decompile(Program program, JsonObject request) throws BridgeException {
		Function function = findFunction(program, request);
		int timeout = boundedInt(request, "timeout", 60, 1, 300);
		DecompInterface decompiler = new DecompInterface();
		try {
			if (!decompiler.openProgram(program)) {
				throw error("decompile_failed", "Decompiler could not open the active program");
			}
			DecompileResults results =
				decompiler.decompileFunction(function, timeout, TaskMonitor.DUMMY);
			if (!results.decompileCompleted() || results.getDecompiledFunction() == null) {
				throw error("decompile_failed",
					"Decompilation failed: " + results.getErrorMessage());
			}
			JsonObject result = new JsonObject();
			result.add("function", functionData(function));
			result.addProperty("c", results.getDecompiledFunction().getC());
			return result;
		}
		finally {
			decompiler.dispose();
		}
	}

	private JsonObject inspectData(Program program, JsonObject request) throws BridgeException {
	Address address = parseAddress(program, requiredString(request, "address")); MemoryBlock block = program.getMemory().getBlock(address);
	if (block == null) throw error("invalid_memory", "Address is not mapped memory: " + address);
	int length = boundedInt(request, "length", 64, 1, 4096); length = (int)Math.min(length, block.getEnd().subtract(address) + 1); byte[] bytes = new byte[length];
	try { program.getMemory().getBytes(address, bytes); } catch (MemoryAccessException e) { throw error("invalid_memory", "Could not read requested memory"); }
	StringBuilder hex = new StringBuilder(), ascii = new StringBuilder(), utf16 = new StringBuilder(); for (int i=0;i<bytes.length;i++) { int b=bytes[i]&255; hex.append(String.format("%02x",b)); ascii.append(b>=32&&b<127?(char)b:'.'); if(i%2==1) { int c=(bytes[i-1]&255)|(b<<8); utf16.append(c>=32&&c<127?(char)c:'.'); } }
	JsonObject result=new JsonObject(); result.addProperty("address",address.toString()); result.addProperty("hex",hex.toString()); result.addProperty("ascii",ascii.toString()); result.addProperty("utf16le",utf16.toString()); result.addProperty("memory_block",block.getName()); result.addProperty("read",block.isRead()); result.addProperty("write",block.isWrite()); result.addProperty("execute",block.isExecute()); result.addProperty("read_only",block.isRead()&&!block.isWrite()); Data data=program.getListing().getDefinedDataAt(address); if(data!=null) result.addProperty("datatype",data.getDataType().getPathName()); result.add("labels",getLabels(program,request).get("labels")); return result;
}
private JsonArray outgoingReferences(Program program, Address address) { JsonArray items=new JsonArray(); for (Reference ref:program.getReferenceManager().getReferencesFrom(address)) { JsonObject item=new JsonObject(); item.addProperty("from",ref.getFromAddress().toString()); item.addProperty("to",ref.getToAddress().toString()); item.addProperty("type",ref.getReferenceType().toString()); item.addProperty("source",ref.getSource().toString()); items.add(item); } return items; }
private JsonObject disassemble(Program program, JsonObject request) throws BridgeException { Address address=parseAddress(program,requiredString(request,"address")); int limit=boundedInt(request,"instruction_count",32,1,512); JsonArray items=new JsonArray(); InstructionIterator it=program.getListing().getInstructions(address,true); while(it.hasNext()&&items.size()<limit) { Instruction ins=it.next(); JsonObject item=new JsonObject(); item.addProperty("address",ins.getAddress().toString()); item.addProperty("mnemonic",ins.getMnemonicString()); item.addProperty("operands",ins.toString()); item.addProperty("flow_type",ins.getFlowType().toString()); item.add("references",outgoingReferences(program,ins.getAddress())); items.add(item); } JsonObject result=new JsonObject(); result.add("instructions",items); return result; }
private JsonObject referencesFrom(Program program, JsonObject request) throws BridgeException { String scope=optionalString(request,"scope","address"); int limit=boundedInt(request,"limit",1000,1,5000); JsonArray items=new JsonArray(); if("function".equals(scope)) { Function f=findFunction(program,request); InstructionIterator it=program.getListing().getInstructions(f.getBody(),true); while(it.hasNext()&&items.size()<limit) for(JsonElement r:outgoingReferences(program,it.next().getAddress())) if(items.size()<limit) items.add(r); } else { for(JsonElement r:outgoingReferences(program,parseAddress(program,requiredString(request,"address")))) if(items.size()<limit) items.add(r); } JsonObject result=new JsonObject(); result.addProperty("scope",scope); result.add("references",items); return result; }
private JsonObject searchStrings(Program program, JsonObject request) throws BridgeException { String query=requiredString(request,"query").toLowerCase(Locale.ROOT), encoding=optionalString(request,"encoding","any"); int limit=boundedInt(request,"limit",100,1,1000); JsonArray items=new JsonArray(); DataIterator it=program.getListing().getDefinedData(true); while(it.hasNext()&&items.size()<limit) { Data d=it.next(); Object v=d.getValue(); if(!(v instanceof String s)||!s.toLowerCase(Locale.ROOT).contains(query)) continue; String detected=d.getDataType().getName().toLowerCase(Locale.ROOT).contains("unicode")?"utf16le":"ascii"; if(!"any".equals(encoding)&&!encoding.equals(detected))continue; JsonObject item=new JsonObject(); item.addProperty("address",d.getAddress().toString()); item.addProperty("value",s); item.addProperty("encoding",detected); item.addProperty("datatype",d.getDataType().getPathName()); item.add("references",outgoingReferences(program,d.getAddress())); items.add(item); } JsonObject result=new JsonObject(); result.add("strings",items); result.addProperty("limit",limit); return result; }
private JsonObject previewMembers(Program program, JsonObject request) throws BridgeException { Composite c=resolveComposite(program,request); JsonArray members=requiredArray(request,"members"), conflicts=new JsonArray(), resolved=new JsonArray(); List<int[]> used=new ArrayList<>(); for(DataTypeComponent x:c.getDefinedComponents())used.add(new int[]{x.getOffset(),x.getLength()}); int required=c.getLength(); for(int i=0;i<members.size();i++){ JsonObject m=members.get(i).getAsJsonObject(); int off=flexibleInteger(m,"offset"), length=has(m,"length")?integer(m,"length"):resolveMemberDataType(program,requiredString(m,"member_data_type")).getLength(), count=has(m,"count")?integer(m,"count"):1, stride=has(m,"stride")?integer(m,"stride"):length; if(off<0||length<1||count<1||stride<1)throw error("invalid_request","Invalid explicit member layout"); for(int n=0;n<count;n++){int at=off+n*stride; JsonObject r=new JsonObject();r.addProperty("offset",at);r.addProperty("length",length);resolved.add(r);for(int[] u:used)if(at<u[0]+u[1]&&u[0]<at+length)conflicts.add(r.deepCopy());used.add(new int[]{at,length});required=Math.max(required,at+length);}} JsonObject result=new JsonObject();result.add("members",resolved);result.add("conflicts",conflicts);result.addProperty("valid",conflicts.size() == 0);result.addProperty("current_size",c.getLength());result.addProperty("required_size",required);result.addProperty("resize_required",required>c.getLength());return result; }
private JsonObject addMembers(Program program, JsonObject request) throws BridgeException { JsonObject preview=previewMembers(program,request); if(!preview.get("valid").getAsBoolean())throw error("layout_conflict","Requested members overlap"); if(preview.get("resize_required").getAsBoolean()&&!booleanValue(request,"allow_resize",false))throw error("layout_conflict","Members require allow_resize=true"); Composite c=resolveComposite(program,request); for(JsonElement e:requiredArray(request,"members")){JsonObject m=e.getAsJsonObject();int count=has(m,"count")?integer(m,"count"):1;int stride=has(m,"stride")?integer(m,"stride"):(has(m,"length")?integer(m,"length"):resolveMemberDataType(program,requiredString(m,"member_data_type")).getLength());for(int n=0;n<count;n++){JsonObject one=m.deepCopy();one.addProperty("data_type",c.getPathName());one.addProperty("offset",flexibleInteger(m,"offset")+n*stride);one.addProperty("allow_resize",booleanValue(request,"allow_resize",false));addMember(program,one);}}return dataTypeMembers(c); }
private JsonObject defineLabeledData(Program program, JsonObject request) throws Exception { Address a=parseAddress(program,requiredString(request,"address")); MemoryBlock b=program.getMemory().getBlock(a);if(b==null)throw error("invalid_memory","Address is not mapped memory");if(b.isExecute())throw error("executable_target","Cannot define data in executable memory");DataType t=resolveMemberDataType(program,requiredString(request,"data_type"));if(t.getLength()<1)throw error("invalid_data_type","Data type must be fixed-size");Listing l=program.getListing();if(l.getInstructionAt(a)!=null)throw error("incompatible_data","Cannot overwrite code");Data d=l.getDefinedDataAt(a);if(d!=null&&!d.getDataType().isEquivalent(t))throw error("incompatible_data","Incompatible defined data");if(d==null)l.createData(a,t);JsonObject label=new JsonObject();label.addProperty("address",a.toString());label.addProperty("label",requiredString(request,"label"));setLabel(program,label);if(has(request,"comment"))l.setComment(a,CodeUnit.EOL_COMMENT,requiredString(request,"comment"));return inspectData(program,label); }
private JsonElement batchMutate(Program program, JsonObject request) throws BridgeException {
	JsonArray operations=requiredArray(request,"operations"); if(operations.size()<1||operations.size()>100)throw error("invalid_batch_operation","operations must contain 1 to 100 entries");
	for(int i=0;i<operations.size();i++){if(!operations.get(i).isJsonObject())throw error("batch_validation_failed","Operation "+i+" must be an object");JsonObject o=operations.get(i).getAsJsonObject();String c=requiredString(o,"command");switch(c){case "rename_function","set_comment"->findFunction(program,o);case "rename_function_parameter"->selectParameter(findFunction(program,o),o);case "rename_local_variable"->selectLocal(findFunction(program,o),o);case "set_function_variable_comment"->selectVariable(findFunction(program,o),o);case "set_function_variable_type"->{selectVariable(findFunction(program,o),o);resolveMemberDataType(program,requiredString(o,"data_type"));}case "set_label","remove_label"->parseAddress(program,requiredString(o,"address"));case "add_data_type_members"->previewMembers(program,o);case "define_labeled_data"->{parseAddress(program,requiredString(o,"address"));resolveMemberDataType(program,requiredString(o,"data_type"));}default->throw error("invalid_batch_operation","Operation "+i+" is not allow-listed: "+c);}}
	return mutate(program,request,"batch_mutate",()->{JsonArray results=new JsonArray();for(int i=0;i<operations.size();i++){JsonObject o=operations.get(i).getAsJsonObject();try{String c=requiredString(o,"command");switch(c){case "rename_function"->{Function f=findFunction(program,o);f.setName(requiredString(o,"new_name"),SourceType.USER_DEFINED);results.add(functionData(f));}case "set_comment"->{Function f=findFunction(program,o);f.setComment(requiredString(o,"comment"));results.add(functionData(f));}case "rename_function_parameter"->{Function f=findFunction(program,o);selectParameter(f,o).setName(requiredString(o,"new_name"),SourceType.USER_DEFINED);results.add(functionVariables(f));}case "rename_local_variable"->{Function f=findFunction(program,o);selectLocal(f,o).setName(requiredString(o,"new_name"),SourceType.USER_DEFINED);results.add(functionVariables(f));}case "set_function_variable_comment"->{Function f=findFunction(program,o);selectVariable(f,o).setComment(requiredString(o,"comment"));results.add(functionVariables(f));}case "set_function_variable_type"->{Function f=findFunction(program,o);selectVariable(f,o).setDataType(resolveMemberDataType(program,requiredString(o,"data_type")),SourceType.USER_DEFINED);results.add(functionVariables(f));}case "set_label"->results.add(setLabel(program,o));case "remove_label"->results.add(removeLabel(program,o));case "add_data_type_members"->results.add(addMembers(program,o));case "define_labeled_data"->results.add(defineLabeledData(program,o));default->throw error("invalid_batch_operation",c);}}catch(Exception e){throw error("batch_failed","Operation "+i+" failed: "+e.getMessage());}}JsonObject result=new JsonObject();result.add("results",results);return result;});
}
private JsonObject referencesTo(Program program, JsonObject request) throws BridgeException {
		Address address = parseAddress(program, requiredString(request, "address"));
		int limit = boundedInt(request, "limit", 100, 1, 1000);
		JsonArray references = new JsonArray();
		ReferenceIterator iterator = program.getReferenceManager().getReferencesTo(address);
		while (iterator.hasNext() && references.size() < limit) {
			Reference reference = iterator.next();
			JsonObject item = new JsonObject();
			item.addProperty("from", reference.getFromAddress().toString());
			item.addProperty("type", reference.getReferenceType().toString());
			item.addProperty("source", reference.getSource().toString());
			references.add(item);
		}
		JsonObject result = new JsonObject();
		result.addProperty("address", address.toString());
		result.add("references", references);
		return result;
	}

	private JsonObject functionVariables(Function function) {
		JsonObject result = new JsonObject();
		result.add("function", functionData(function));
		JsonArray parameters = new JsonArray();
		Parameter[] functionParameters = function.getParameters();
		for (int index = 0; index < functionParameters.length; index++) {
			JsonObject item = variableData(functionParameters[index]);
			item.addProperty("parameter_index", index);
			item.addProperty("ordinal", functionParameters[index].getOrdinal());
			parameters.add(item);
		}
		JsonArray locals = new JsonArray();
		for (Variable variable : function.getLocalVariables()) {
			locals.add(variableData(variable));
		}
		result.add("parameters", parameters);
		result.add("local_variables", locals);
		return result;
	}

	private JsonObject variableData(Variable variable) {
		JsonObject item = new JsonObject();
		item.addProperty("name", variable.getName());
		item.addProperty("datatype", variable.getDataType().getPathName());
		item.addProperty("storage", variable.getVariableStorage().toString());
		if (variable.isStackVariable()) {
			item.addProperty("stack_offset", variable.getStackOffset());
		}
		String comment = variable.getComment();
		if (comment != null) {
			item.addProperty("comment", comment);
		}
		return item;
	}

	private Parameter selectParameter(Function function, JsonObject request) throws BridgeException {
		Parameter[] parameters = function.getParameters();
		if (has(request, "parameter_index")) {
			int index = integer(request, "parameter_index");
			if (index < 0 || index >= parameters.length) {
				throw error("target_not_found", "Parameter index is out of range: " + index);
			}
			return parameters[index];
		}
		String oldName = requiredString(request, "old_name");
		List<Parameter> matches = new ArrayList<>();
		for (Parameter parameter : parameters) {
			if (parameter.getName().equals(oldName)) {
				matches.add(parameter);
			}
		}
		return exactlyOne(matches, "parameter named " + oldName);
	}

	private Variable selectLocal(Function function, JsonObject request) throws BridgeException {
		if (!has(request, "old_name") && !has(request, "storage") &&
			!has(request, "stack_offset")) {
			throw error("invalid_selector",
				"Select a local with old_name, storage, or stack_offset");
		}
		List<Variable> matches = new ArrayList<>();
		for (Variable local : function.getLocalVariables()) {
			boolean match = !has(request, "old_name") ||
				local.getName().equals(requiredString(request, "old_name"));
			match &= !has(request, "storage") ||
				local.getVariableStorage().toString().equals(requiredString(request, "storage"));
			match &= !has(request, "stack_offset") ||
				(local.isStackVariable() && local.getStackOffset() == flexibleInteger(request,
					"stack_offset"));
			if (match) {
				matches.add(local);
			}
		}
		return exactlyOne(matches, "local variable");
	}

	private Variable selectVariable(Function function, JsonObject request) throws BridgeException {
		String type = requiredString(request, "variable_type");
		return switch (type) {
			case "parameter" -> selectParameter(function, request);
			case "local" -> selectLocal(function, request);
			default -> throw error("invalid_selector",
				"variable_type must be 'parameter' or 'local'");
		};
	}

	private Composite resolveComposite(Program program, JsonObject request) throws BridgeException {
		String requested = requiredString(request, "data_type");
		DataTypeManager manager = program.getDataTypeManager();
		DataType found = null;
		if (requested.startsWith("/")) {
			int slash = requested.lastIndexOf('/');
			if (slash >= 0 && slash < requested.length() - 1) {
				found = manager.getDataType(new CategoryPath(
					slash == 0 ? "/" : requested.substring(0, slash)),
					requested.substring(slash + 1));
			}
		}
		else {
			List<DataType> matches = new ArrayList<>();
			manager.findDataTypes(requested, matches);
			List<DataType> exact = matches.stream()
				.filter(dataType -> dataType.getName().equals(requested) ||
					dataType.getPathName().equals(requested))
				.toList();
			if (exact.size() > 1) {
				throw error("ambiguous_target",
					"Data type name is ambiguous; use its full /category/path");
			}
			if (exact.size() == 1) {
				found = exact.get(0);
			}
		}
		if (!(found instanceof Structure) && !(found instanceof Union)) {
			throw error("target_not_found",
				"No structure or union data type matches " + requested);
		}
		return (Composite) found;
	}

	private DataType resolveMemberDataType(Program program, String requested) throws BridgeException {
		JsonObject selector = new JsonObject();
		selector.addProperty("data_type", requested);
		try {
			return resolveAnyDataType(program, requested);
		}
		catch (BridgeException missing) {
			throw error("target_not_found", "Member data type not found: " + requested);
		}
	}

	private DataType resolveAnyDataType(Program program, String requested) throws BridgeException {
		DataTypeManager manager = program.getDataTypeManager();
		if (requested.startsWith("/")) {
			int slash = requested.lastIndexOf('/');
			DataType result = manager.getDataType(
				new CategoryPath(slash == 0 ? "/" : requested.substring(0, slash)),
				requested.substring(slash + 1));
			if (result != null) {
				return result;
			}
		}
		List<DataType> matches = new ArrayList<>();
		manager.findDataTypes(requested, matches);
		List<DataType> exact = matches.stream()
			.filter(dataType -> dataType.getName().equals(requested) ||
				dataType.getPathName().equals(requested))
			.toList();
		if (exact.size() != 1) {
			throw error(exact.isEmpty() ? "target_not_found" : "ambiguous_target",
				"Data type must resolve unambiguously: " + requested);
		}
		return exact.get(0);
	}

	private JsonObject dataTypeMembers(Composite composite) {
		JsonObject result = new JsonObject();
		result.addProperty("name", composite.getName());
		result.addProperty("path", composite.getPathName());
		result.addProperty("size", composite.getLength());
		result.addProperty("kind", composite instanceof Structure ? "struct" : "union");
		JsonArray members = new JsonArray();
		for (DataTypeComponent component : composite.getDefinedComponents()) {
			JsonObject item = new JsonObject();
			item.addProperty("ordinal", component.getOrdinal());
			item.addProperty("offset", component.getOffset());
			item.addProperty("length", component.getLength());
			if (component.getFieldName() != null) {
				item.addProperty("field_name", component.getFieldName());
			}
			item.addProperty("datatype", component.getDataType().getPathName());
			if (component.getComment() != null) {
				item.addProperty("comment", component.getComment());
			}
			members.add(item);
		}
		result.add("members", members);
		return result;
	}
	private JsonObject createDataType(Program program, JsonObject request) throws BridgeException {
		String name = requiredString(request, "name");
		String kind = requiredString(request, "kind");
		String categoryText = optionalString(request, "category_path", "/");
		if (!categoryText.startsWith("/")) {
			throw error("invalid_request", "category_path must be an absolute Ghidra category path");
		}
		CategoryPath category;
		try {
			category = new CategoryPath(categoryText);
		}
		catch (IllegalArgumentException invalidPath) {
			throw error("invalid_request", "Invalid category_path: " + categoryText);
		}
		DataTypeManager manager = program.getDataTypeManager();
		if (manager.getDataType(category, name) != null) {
			throw error("data_type_exists", "Data type already exists: " + category.getPath() + "/" + name);
		}
		try {
			if (manager.getCategory(category) == null) {
				manager.createCategory(category);
			}
		}
		catch (Exception createCategoryError) {
			throw error("request_failed", "Could not create category " + category.getPath());
		}
		Composite created = switch (kind) {
			case "struct" -> new StructureDataType(category, name,
				has(request, "size") ? integer(request, "size") : 0, manager);
			case "union" -> new UnionDataType(category, name, manager);
			default -> throw error("invalid_request", "kind must be 'struct' or 'union'");
		};
		DataType added = manager.addDataType(created, DataTypeConflictHandler.DEFAULT_HANDLER);
		if (!(added instanceof Composite composite)) {
			throw error("request_failed", "Ghidra did not create a composite data type");
		}
		return dataTypeMembers(composite);
	}


	private DataTypeComponent selectComponent(Composite composite, JsonObject request,
			String nameKey) throws BridgeException {
		List<DataTypeComponent> matches = new ArrayList<>();
		for (DataTypeComponent component : composite.getDefinedComponents()) {
			boolean match = true;
			if (has(request, "ordinal")) {
				match &= component.getOrdinal() == integer(request, "ordinal");
			}
			if (has(request, "offset")) {
				match &= component.getOffset() == flexibleInteger(request, "offset");
			}
			if (has(request, nameKey)) {
				match &= Objects.equals(component.getFieldName(), requiredString(request, nameKey));
			}
			if (match) {
				matches.add(component);
			}
		}
		if (!has(request, "ordinal") && !has(request, "offset") && !has(request, nameKey)) {
			throw error("invalid_selector",
				"Select a member with offset, ordinal, or " + nameKey);
		}
		return exactlyOne(matches, "data type member");
	}

	private JsonObject setMemberType(Program program, JsonObject request) throws BridgeException {
		Composite composite = resolveComposite(program, request);
		DataTypeComponent old = selectComponent(composite, request, "field_name");
		DataType replacement =
			resolveMemberDataType(program, requiredString(request, "member_data_type"));
		boolean allowResize = booleanValue(request, "allow_resize", false);
		int naturalLength = replacement.getLength();
		if (naturalLength < 1) {
			throw error("invalid_data_type",
				"Dynamic-length member types require an explicit supported allocation");
		}
		int allocation = allowResize ? naturalLength : old.getLength();
		if (!allowResize && naturalLength > old.getLength()) {
			throw error("layout_conflict", "New member type requires " + naturalLength +
				" bytes but the existing allocation is " + old.getLength());
		}
		if (composite instanceof Structure structure) {
			if (structure.isPackingEnabled() && !allowResize &&
				naturalLength != old.getLength()) {
				throw error("layout_conflict",
					"Changing allocation in a packed structure requires allow_resize=true");
			}
			ensureNoOverlap(structure, old, old.getOffset(), allocation);
			structure.replaceAtOffset(old.getOffset(), replacement, allocation,
				old.getFieldName(), old.getComment());
		}
		else {
			Union union = (Union) composite;
			int ordinal = old.getOrdinal();
			String fieldName = old.getFieldName();
			String comment = old.getComment();
			union.delete(ordinal);
			union.insert(ordinal, replacement, allocation, fieldName, comment);
		}
		return dataTypeMembers(composite);
	}

	private JsonObject addMember(Program program, JsonObject request) throws BridgeException {
		Composite composite = resolveComposite(program, request);
		DataType dataType =
			resolveMemberDataType(program, requiredString(request, "member_data_type"));
		String fieldName = requiredString(request, "field_name");
		String comment = optionalNullableString(request, "comment");
		int naturalLength = dataType.getLength();
		int length = has(request, "length") ? integer(request, "length") : naturalLength;
		if (length < 1) {
			throw error("invalid_data_type", "A positive member length is required");
		}
		if (composite instanceof Union union) {
			union.add(dataType, length, fieldName, comment);
			return dataTypeMembers(composite);
		}
		Structure structure = (Structure) composite;
		boolean append = booleanValue(request, "append", false);
		boolean allowResize = booleanValue(request, "allow_resize", false);
		if (structure.isPackingEnabled() && !allowResize) {
			throw error("layout_conflict",
				"Adding to a packed structure requires allow_resize=true because offsets may shift");
		}
		if (append) {
			if (has(request, "offset")) {
				throw error("invalid_request", "Do not combine append=true with offset");
			}
			structure.add(dataType, length, fieldName, comment);
			return dataTypeMembers(composite);
		}
		if (!has(request, "offset")) {
			throw error("invalid_request",
				"Structures require offset or an explicit append=true");
		}
		int offset = flexibleInteger(request, "offset");
		if (offset < 0) {
			throw error("invalid_request", "offset must not be negative");
		}
		if (offset > structure.getLength()) {
			if (!allowResize) {
				throw error("layout_conflict",
					"Offset is beyond the structure; allow_resize=true is required");
			}
			structure.growStructure(offset - structure.getLength());
		}
		ensureNoOverlap(structure, null, offset, length);
		if (offset == structure.getLength()) {
			structure.add(dataType, length, fieldName, comment);
		}
		else {
			if (offset + length > structure.getLength() && !allowResize) {
				throw error("layout_conflict",
					"Member exceeds the structure; allow_resize=true is required");
			}
			structure.replaceAtOffset(offset, dataType, length, fieldName, comment);
		}
		return dataTypeMembers(composite);
	}

	private void ensureNoOverlap(Structure structure, DataTypeComponent replaced, int offset,
			int length) throws BridgeException {
		long end = (long) offset + length;
		for (DataTypeComponent component : structure.getDefinedComponents()) {
			if (replaced != null && component.getOrdinal() == replaced.getOrdinal()) {
				continue;
			}
			long componentEnd = (long) component.getOffset() + component.getLength();
			if (offset < componentEnd && component.getOffset() < end) {
				throw error("layout_conflict",
					"Requested allocation overlaps member ordinal " + component.getOrdinal());
			}
		}
	}

	private JsonObject getLabels(Program program, JsonObject request) throws BridgeException {
		Address address = parseAddress(program, requiredString(request, "address"));
		JsonArray labels = new JsonArray();
		for (Symbol symbol : program.getSymbolTable().getSymbols(address)) {
			JsonObject item = new JsonObject();
			item.addProperty("name", symbol.getName());
			item.addProperty("namespace", symbol.getParentNamespace().getName(true));
			item.addProperty("source", symbol.getSource().toString());
			item.addProperty("type", symbol.getSymbolType().toString());
			item.addProperty("primary", symbol.isPrimary());
			labels.add(item);
		}
		JsonObject result = new JsonObject();
		result.addProperty("address", address.toString());
		result.add("labels", labels);
		return result;
	}

	private JsonObject setLabel(Program program, JsonObject request) throws Exception {
		Address address = parseAddress(program, requiredString(request, "address"));
		String name = requiredString(request, "label");
		Namespace namespace = resolveNamespace(program, optionalNullableString(request, "namespace"));
		boolean replace = booleanValue(request, "replace", false);
		Symbol existingUserLabel = null;
		for (Symbol symbol : program.getSymbolTable().getSymbols(address)) {
			if (!symbol.getName().equals(name) ||
				!symbol.getParentNamespace().equals(namespace)) {
				continue;
			}
			if (symbol.getSymbolType() == SymbolType.FUNCTION) {
				throw error("protected_symbol",
					"Function symbols cannot be replaced through set_label");
			}
			if (symbol.getSymbolType() == SymbolType.LABEL &&
				symbol.getSource() == SourceType.USER_DEFINED) {
				existingUserLabel = symbol;
				continue;
			}
			if (!replace) {
				throw error("symbol_exists",
					"A non-user symbol already has that name; pass replace=true to replace it");
			}
			if (!symbol.delete()) {
				throw error("protected_symbol", "Existing symbol could not be replaced");
			}
		}
		if (existingUserLabel == null) {
			program.getSymbolTable().createLabel(address, name, namespace,
				SourceType.USER_DEFINED);
		}
		return getLabels(program, request);
	}

	private JsonObject removeLabel(Program program, JsonObject request) throws BridgeException {
		Address address = parseAddress(program, requiredString(request, "address"));
		String name = requiredString(request, "label");
		Namespace namespace = resolveNamespace(program, optionalNullableString(request, "namespace"));
		List<Symbol> matches = new ArrayList<>();
		for (Symbol symbol : program.getSymbolTable().getSymbols(address)) {
			if (symbol.getName().equals(name) &&
				symbol.getParentNamespace().equals(namespace)) {
				matches.add(symbol);
			}
		}
		Symbol symbol = exactlyOne(matches, "label");
		if (symbol.getSymbolType() != SymbolType.LABEL ||
			symbol.getSource() != SourceType.USER_DEFINED) {
			throw error("protected_symbol",
				"Only user-defined label symbols may be removed");
		}
		if (!symbol.delete()) {
			throw error("request_failed", "Ghidra refused to remove the label");
		}
		return getLabels(program, request);
	}

	private Namespace resolveNamespace(Program program, String path) throws BridgeException {
		Namespace namespace = program.getGlobalNamespace();
		if (path == null || path.isBlank() || path.equals("Global")) {
			return namespace;
		}
		String normalized = path.startsWith("Global::") ? path.substring(8) : path;
		for (String part : normalized.split("::")) {
			if (part.isBlank()) {
				throw error("invalid_namespace", "Invalid namespace path: " + path);
			}
			Namespace next = program.getSymbolTable().getNamespace(part, namespace);
			if (next == null) {
				throw error("target_not_found", "Namespace does not exist: " + path);
			}
			namespace = next;
		}
		return namespace;
	}

	private JsonElement mutate(Program program, JsonObject request, String command,
			Mutation mutation) throws BridgeException {
		verifyIdentity(program, request);
		DomainFile file = program.getDomainFile();
		if (file == null) {
			throw error("unsavable_program",
				"The active program has no associated project file");
		}
		if (!file.isInWritableProject() || file.isReadOnly() || !program.canSave()) {
			throw error("read_only", "The active program is read-only or cannot be saved");
		}
		int transaction = program.startTransaction("Totala MCP: " + command);
		boolean commit = false;
		JsonElement result;
		try {
			result = mutation.run();
			commit = true;
		}
		catch (BridgeException bridgeError) {
			throw bridgeError;
		}
		catch (Exception requestError) {
			String message = requestError.getMessage();
			throw error("mutation_failed",
				message == null ? requestError.getClass().getSimpleName() : message);
		}
		finally {
			program.endTransaction(transaction, commit);
		}
		try {
			program.save("Totala MCP committed " + command, TaskMonitor.DUMMY);
		}
		catch (Exception saveError) {
			throw error("save_failed",
				"Mutation committed in memory but the project save failed: " +
					saveError.getMessage());
		}
		if (result.isJsonObject()) {
			result.getAsJsonObject().addProperty("persisted", true);
		}
		return result;
	}

	private void verifyIdentity(Program program, JsonObject request) throws BridgeException {
		verifyMatch(request, "program", program.getName());
		verifyMatch(request, "executable_path", program.getExecutablePath());
		verifyMatch(request, "image_base", program.getImageBase().toString());
	}

	private void verifyMatch(JsonObject request, String key, String actual) throws BridgeException {
		if (has(request, key) && !requiredString(request, key).equals(actual)) {
			throw error("program_mismatch",
				key + " does not match the active program (expected " + actual + ")");
		}
	}

	private Function findFunction(Program program, JsonObject request) throws BridgeException {
		if (has(request, "address")) {
			String text = requiredString(request, "address");
			Function function = program.getFunctionManager().getFunctionContaining(
				parseAddress(program, text));
			if (function == null) {
				throw error("target_not_found", "No function contains " + text);
			}
			return function;
		}
		String name = requiredString(request, "name");
		List<Function> matches = new ArrayList<>();
		FunctionIterator iterator = program.getFunctionManager().getFunctions(true);
		while (iterator.hasNext()) {
			Function candidate = iterator.next();
			if (candidate.getName().equals(name)) {
				matches.add(candidate);
			}
		}
		if (matches.isEmpty()) {
			throw error("target_not_found", "No function named " + name);
		}
		if (matches.size() > 1) {
			throw error("ambiguous_target",
				"Multiple functions are named " + name + "; use address");
		}
		return matches.get(0);
	}

	private Address parseAddress(Program program, String text) throws BridgeException {
		Address address = program.getAddressFactory().getAddress(text);
		if (address == null && text.toLowerCase(Locale.ROOT).startsWith("0x")) {
			try {
				long offset = Long.parseUnsignedLong(text.substring(2), 16);
				address = program.getAddressFactory().getDefaultAddressSpace().getAddress(offset);
			}
			catch (NumberFormatException | AddressOutOfBoundsException ignored) {
				// Report the common structured error below.
			}
		}
		if (address == null) {
			throw error("invalid_address", "Invalid program address: " + text);
		}
		return address;
	}

	private static <T> T exactlyOne(List<T> matches, String description)
			throws BridgeException {
		if (matches.isEmpty()) {
			throw error("target_not_found", "No matching " + description);
		}
		if (matches.size() > 1) {
			throw error("ambiguous_target", "Multiple matches for " + description);
		}
		return matches.get(0);
	}

	private static JsonArray requiredArray(JsonObject object, String key) throws BridgeException { if (!has(object, key) || !object.get(key).isJsonArray()) throw error("invalid_request", key + " is required and must be an array"); return object.getAsJsonArray(key); }

	private static boolean has(JsonObject object, String key) {
		return object.has(key) && !object.get(key).isJsonNull();
	}

	private static String requiredString(JsonObject object, String key) throws BridgeException {
		if (!has(object, key) || !object.get(key).isJsonPrimitive() ||
			!object.get(key).getAsJsonPrimitive().isString()) {
			throw error("invalid_request", key + " is required and must be a string");
		}
		String value = object.get(key).getAsString();
		if (value.isEmpty() && !key.equals("comment")) {
			throw error("invalid_request", key + " must not be empty");
		}
		return value;
	}

	private static String optionalString(JsonObject object, String key, String fallback) {
		return has(object, key) ? object.get(key).getAsString() : fallback;
	}

	private static String optionalNullableString(JsonObject object, String key) {
		return has(object, key) ? object.get(key).getAsString() : null;
	}

	private static int boundedInt(JsonObject object, String key, int fallback, int minimum,
			int maximum) {
		int value = has(object, key) ? object.get(key).getAsInt() : fallback;
		return Math.min(Math.max(value, minimum), maximum);
	}

	private static int integer(JsonObject object, String key) throws BridgeException {
		try {
			return object.get(key).getAsInt();
		}
		catch (Exception error) {
			throw error("invalid_request", key + " must be an integer");
		}
	}

	private static int flexibleInteger(JsonObject object, String key) throws BridgeException {
		try {
			JsonElement value = object.get(key);
			if (value.getAsJsonPrimitive().isNumber()) {
				return value.getAsInt();
			}
			String text = value.getAsString();
			boolean negative = text.startsWith("-");
			String unsigned = negative ? text.substring(1) : text;
			int radix = unsigned.toLowerCase(Locale.ROOT).startsWith("0x") ? 16 : 10;
			if (radix == 16) {
				unsigned = unsigned.substring(2);
			}
			int parsed = Integer.parseUnsignedInt(unsigned, radix);
			return negative ? -parsed : parsed;
		}
		catch (Exception error) {
			throw error("invalid_request", key + " must be an integer or 0x-prefixed integer");
		}
	}

	private static boolean booleanValue(JsonObject object, String key, boolean fallback) {
		return has(object, key) ? object.get(key).getAsBoolean() : fallback;
	}

	private static BridgeException error(String code, String message) {
		return new BridgeException(code, message);
	}

	@FunctionalInterface
	private interface Mutation {
		JsonElement run() throws Exception;
	}
}
