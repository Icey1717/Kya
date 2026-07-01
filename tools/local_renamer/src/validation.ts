import { z } from "zod";
import type { Declaration, NormalizedResponse, Suggestion } from "./types.js";

const cppKeywords = new Set(
  (
    "alignas alignof and and_eq asm atomic_cancel atomic_commit atomic_noexcept auto bitand bitor bool break " +
    "case catch char char8_t char16_t char32_t class compl concept const consteval constexpr constinit const_cast " +
    "continue co_await co_return co_yield decltype default delete do double dynamic_cast else enum explicit export " +
    "extern false float for friend goto if import inline int long module mutable namespace new noexcept not not_eq nullptr operator " +
    "or or_eq private protected public reflexpr register reinterpret_cast requires return short signed sizeof static " +
    "static_assert static_cast struct switch synchronized template this thread_local throw true try typedef typeid " +
    "typename union unsigned using virtual void volatile wchar_t while xor xor_eq"
  ).split(/\s+/)
);

const suggestionSchema = z
  .object({
    declaration_id: z.string(),
    old_name: z.string(),
    suggested_name: z.string(),
    confidence: z.number().min(0).max(1)
  })
  .strict();

const mapSuggestionSchema = suggestionSchema.omit({ declaration_id: true });
const responseSchema = z.object({ suggestions: z.array(suggestionSchema) }).strict();
const responseMapSchema = z.record(z.string(), mapSuggestionSchema);

export function parseStrictJsonResponse(raw: string): unknown {
  const trimmed = raw.trim();
  if (trimmed.startsWith("{") && trimmed.endsWith("}")) return JSON.parse(trimmed);
  const match = trimmed.match(/^\`\`\`(?:json)?\s*\r?\n([\s\S]*?)\r?\n\`\`\`$/i);
  if (!match) throw new Error("Response must be raw JSON or one JSON Markdown fence.");
  return JSON.parse(match[1]);
}

export function validateChangedName(name: string, declaration: Pick<Declaration, "isPointer" | "isBoolean">): string[] {
  const errors: string[] = [];
  if (!/^[A-Za-z][A-Za-z0-9]*$/.test(name)) errors.push("not a valid C++ identifier");
  if (cppKeywords.has(name)) errors.push("is a C++ keyword");
  if (!/^[a-z]/.test(name)) errors.push("must use lower camel case");
  if (name.includes("_")) errors.push("must not contain underscores");
  if (/[A-Z]{2}/.test(name)) errors.push("must not contain adjacent uppercase letters");
  if (/\d$/.test(name)) errors.push("must not end in a digit");
  if (declaration.isPointer && !/^p[A-Z]/.test(name)) errors.push("pointer names must start with p followed by uppercase");
  if (!declaration.isPointer && declaration.isBoolean && !/^b[A-Z]/.test(name)) {
    errors.push("Boolean names must start with b followed by uppercase");
  }
  return errors;
}

export function validateAndNormalizeResponse(
  raw: string,
  declarations: Declaration[],
  parameters: string[]
): NormalizedResponse {
  const parsedRaw = parseStrictJsonResponse(raw);
  const parsed = responseSchema.safeParse(parsedRaw);
  const suggestions = parsed.success
    ? parsed.data.suggestions
    : Object.entries(responseMapSchema.parse(parsedRaw) as Record<string, { old_name: string; suggested_name: string; confidence: number }>).map(
        ([declaration_id, suggestion]) => ({
          declaration_id,
          old_name: suggestion.old_name,
          suggested_name: suggestion.suggested_name,
          confidence: suggestion.confidence
        })
      );

  if (suggestions.length !== declarations.length) {
    throw new Error("Response must contain exactly one entry per declaration.");
  }

  const inventory = new Map(declarations.map((declaration) => [declaration.id, declaration]));
  const seen = new Set<string>();
  const normalizedById = new Map<string, Suggestion & { decision: string; keep: boolean }>();

  for (const suggestion of suggestions) {
    const declaration = inventory.get(suggestion.declaration_id);
    if (!declaration) throw new Error(`Unknown declaration ID: ${suggestion.declaration_id}`);
    if (seen.has(suggestion.declaration_id)) throw new Error(`Duplicate declaration ID: ${suggestion.declaration_id}`);
    seen.add(suggestion.declaration_id);
    if (suggestion.old_name !== declaration.oldName) {
      throw new Error(`old_name mismatch for ${suggestion.declaration_id}`);
    }
    const keep = suggestion.suggested_name === suggestion.old_name;
    if (!keep) {
      const errors = validateChangedName(suggestion.suggested_name, declaration);
      if (errors.length > 0) throw new Error(`Invalid name ${suggestion.suggested_name}: ${errors.join(", ")}`);
    }
    normalizedById.set(suggestion.declaration_id, {
      ...suggestion,
      decision: keep ? "keep" : suggestion.suggested_name,
      keep
    });
  }

  const finalNames = new Map<string, Array<{ original: string; keep: boolean }>>();
  for (const parameter of parameters) {
    finalNames.set(parameter, [...(finalNames.get(parameter) ?? []), { original: parameter, keep: true }]);
  }
  for (const declaration of declarations) {
    const suggestion = normalizedById.get(declaration.id);
    if (!suggestion) throw new Error(`Missing declaration ID: ${declaration.id}`);
    const finalName = suggestion.keep ? declaration.oldName : suggestion.suggested_name;
    finalNames.set(finalName, [
      ...(finalNames.get(finalName) ?? []),
      { original: declaration.oldName, keep: suggestion.keep }
    ]);
  }
  for (const [name, entries] of finalNames) {
    if (entries.length > 1 && !entries.every((entry) => entry.keep && entry.original === name)) {
      throw new Error(`Function-wide name collision: ${name}`);
    }
  }

  return { suggestions: declarations.map((declaration) => normalizedById.get(declaration.id)!) };
}

export function suggestionJsonSchema(declarations: Declaration[]): object {
  return {
    type: "object",
    additionalProperties: false,
    properties: {
      suggestions: {
        type: "array",
        minItems: declarations.length,
        maxItems: declarations.length,
        items: {
          type: "object",
          additionalProperties: false,
          properties: {
            declaration_id: { type: "string" },
            old_name: { type: "string" },
            suggested_name: { type: "string" },
            confidence: { type: "number", minimum: 0, maximum: 1 }
          },
          required: ["declaration_id", "old_name", "suggested_name", "confidence"]
        }
      }
    },
    required: ["suggestions"]
  };
}