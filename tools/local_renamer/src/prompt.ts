import type { FunctionRecord } from "./types.js";

export function outputTokenLimit(declarationCount: number): number {
  return Math.min(8192, Math.max(512, 64 * declarationCount));
}

export function buildPrompt(fn: FunctionRecord): string {
  const inventory = fn.declarations.map((declaration) => ({
    declaration_id: declaration.id,
    old_name: declaration.oldName,
    declared_type: declaration.declaredType
  }));
  return [
    "Suggest a name decision for every listed C++ local declaration.",
    "Return only one JSON object in exactly this shape:",
    '{"suggestions":[{"declaration_id":"<exact inventory declaration_id>","old_name":"<exact inventory old_name>","suggested_name":"<chosen name>","confidence":0.5}]}',
    "Include exactly one suggestions entry per declaration. Do not return a keyed map or a top-level array.",
    "confidence is required and must be a JSON number from 0 through 1.",
    "Use the exact declaration_id and old_name. To keep a name, repeat old_name as suggested_name.",
    "Changed names must be valid lower-camel-case C++ identifiers with no underscores.",
    "Pointer names start with p followed by uppercase. Non-pointer Boolean names start with b followed by uppercase.",
    "Do not rename parameters, members, globals, macros, types, labels, or anything outside the inventory.",
    "Do not include rationale, commentary, or fields other than those shown in the required shape.",
    "",
    "Declaration inventory:",
    JSON.stringify(inventory),
    "",
    "Complete function definition:",
    fn.promptSource ?? fn.source
  ].join("\n");
}

export function estimatePromptTokens(fn: FunctionRecord): number {
  // OpenRouter models use different tokenizers. UTF-8 byte count plus chat
  // framing is a tokenizer-independent upper bound for this source-oriented
  // prompt and intentionally favors exclusion over truncation.
  return Buffer.byteLength(buildPrompt(fn), "utf8") + 256;
}
