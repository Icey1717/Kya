import { createHash, randomInt } from "node:crypto";
import { identifierOccurrences } from "./cppAnalyzer.js";
import { requestKey } from "./runFactory.js";
import type {
  EvaluationRun,
  PublicReviewItem,
  ReviewItem,
  ReviewOptionInternal,
  ReviewVote
} from "./types.js";
import { validateChangedName } from "./validation.js";

function shuffle<T>(values: T[]): T[] {
  const result = [...values];
  for (let index = result.length - 1; index > 0; index--) {
    const other = randomInt(index + 1);
    [result[index], result[other]] = [result[other], result[index]];
  }
  return result;
}

function label(index: number): string {
  return `answer_${String(index + 1).padStart(2, "0")}`;
}

export function buildReviewItems(run: EvaluationRun): ReviewItem[] {
  const items: ReviewItem[] = [];
  for (const file of run.manifest.files) {
    if (!file.reviewAvailable) continue;
    for (const functionId of file.randomizedFunctionIds) {
      const fn = file.functions.find((candidate) => candidate.id === functionId);
      if (!fn) continue;
      const requests = run.manifest.modelIds
        .map((modelId) => run.requests[requestKey(fn.id, modelId)])
        .filter((request) => request?.status === "success");
      if (requests.length === 0) continue;
      for (const declaration of fn.declarations) {
        const decisions = new Map<string, Omit<ReviewOptionInternal, "label">>();
        decisions.set("keep", { decision: "keep", modelIds: [], confidences: [] });
        for (const request of requests) {
          const suggestion = request.normalized?.suggestions.find(
            (candidate) => candidate.declaration_id === declaration.id
          );
          if (!suggestion) continue;
          const existing = decisions.get(suggestion.decision) ?? {
            decision: suggestion.decision,
            modelIds: [],
            confidences: []
          };
          existing.modelIds.push(request.modelId);
          existing.confidences.push(suggestion.confidence);
          decisions.set(suggestion.decision, existing);
        }
        const options = shuffle([...decisions.values()]).map((option, index) => ({
          ...option,
          label: label(index)
        }));
        items.push({
          key: createHash("sha256").update(`${fn.id}\0${declaration.id}`).digest("hex").slice(0, 24),
          filePath: file.relativePath,
          functionId: fn.id,
          functionName: fn.qualifiedName,
          declarationId: declaration.id,
          oldName: declaration.oldName,
          options
        });
      }
    }
  }
  return items;
}

export function publicReviewItems(run: EvaluationRun): PublicReviewItem[] {
  return run.reviewItems.map((item) => {
    const file = run.manifest.files.find((candidate) => candidate.relativePath === item.filePath);
    const fn = file?.functions.find((candidate) => candidate.id === item.functionId);
    const declaration = fn?.declarations.find((candidate) => candidate.id === item.declarationId);
    const highlightRanges = fn && declaration
      ? declaration.occurrences?.length
        ? declaration.occurrences.map((occurrence) => ({
            start: occurrence.startIndex - fn.startIndex,
            end: occurrence.endIndex - fn.startIndex
          }))
        : identifierOccurrences(fn.source, declaration.oldName).map((occurrence) => ({
            start: occurrence.startIndex,
            end: occurrence.endIndex
          }))
      : [];
    return {
      key: item.key,
      filePath: item.filePath,
      functionId: item.functionId,
      functionName: item.functionName,
      declarationId: item.declarationId,
      oldName: item.oldName,
      functionSource: fn?.source ?? "",
      highlightRanges,
      vote: item.vote,
      options: item.options.map((option) => ({
        label: option.label,
        decision: option.decision,
        ...(item.vote ? { confidences: option.confidences } : {})
      }))
    };
  });
}

export function submitVote(
  run: EvaluationRun,
  itemKey: string,
  input: { preferredLabel?: string; acceptableLabels?: string[]; customName?: string }
): ReviewVote {
  const item = run.reviewItems.find((candidate) => candidate.key === itemKey);
  if (!item) throw new Error("Unknown review item.");
  if (item.vote) throw new Error("This vote is permanently locked.");
  const labels = new Set(item.options.map((option) => option.label));
  const acceptableLabels = [...new Set(input.acceptableLabels ?? [])];
  if (input.customName) {
    if (input.preferredLabel || acceptableLabels.length > 0) {
      throw new Error("A reviewer-authored name cannot be combined with model answers.");
    }
    if (input.customName === item.oldName) throw new Error("Select keep existing instead of entering the old name.");
    const file = run.manifest.files.find((candidate) => candidate.relativePath === item.filePath);
    const fn = file?.functions.find((candidate) => candidate.id === item.functionId);
    const declaration = fn?.declarations.find((candidate) => candidate.id === item.declarationId);
    if (!fn || !declaration) throw new Error("Review declaration is missing from the manifest.");
    const errors = validateChangedName(input.customName, declaration);
    if (errors.length > 0) throw new Error(`Invalid reviewer name: ${errors.join(", ")}`);
    const occupied = new Set([
      ...fn.parameters,
      ...fn.declarations.filter((candidate) => candidate.id !== declaration.id).map((candidate) => candidate.oldName)
    ]);
    if (occupied.has(input.customName)) throw new Error(`Function-wide name collision: ${input.customName}`);
  } else {
    if (!input.preferredLabel || !labels.has(input.preferredLabel)) throw new Error("Select one preferred answer.");
    if (acceptableLabels.some((candidate) => !labels.has(candidate))) throw new Error("Unknown acceptable answer.");
    acceptableLabels.splice(
      0,
      acceptableLabels.length,
      ...acceptableLabels.filter((candidate) => candidate !== input.preferredLabel)
    );
  }
  const vote: ReviewVote = {
    preferredLabel: input.customName ? undefined : input.preferredLabel,
    acceptableLabels,
    customName: input.customName,
    submittedAt: new Date().toISOString()
  };
  item.vote = vote;
  return vote;
}
