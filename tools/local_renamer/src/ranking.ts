import { requestKey } from "./runFactory.js";
import type { EvaluationRun, ModelRanking, RankingReport, ReviewItem } from "./types.js";

function modelCost(run: EvaluationRun, modelId: string): ModelRanking["cost"] {
  const metadata = run.manifest.models.find((model) => model.id === modelId);
  const requests = Object.values(run.requests).filter((request) => request.modelId === modelId);
  const attempts = requests.flatMap((request) => request.attempts);
  let total = 0;
  let estimated = false;
  for (const attempt of attempts) {
    if (attempt.cost !== undefined) {
      total += attempt.cost;
      continue;
    }
    const estimate = (attempt.promptTokens ?? 0) * (metadata?.promptPrice ?? 0)
      + (attempt.completionTokens ?? 0) * (metadata?.completionPrice ?? 0);
    total += estimate;
    if (estimate > 0) estimated = true;
  }
  const successfulAnswers = requests.filter((request) => request.status === "success").length;
  return {
    total,
    perSuccessfulAnswer: successfulAnswers ? total / successfulAnswers : 0,
    estimated
  };
}

export function withCostSummaries(run: EvaluationRun, ranking: RankingReport): RankingReport {
  return {
    ...ranking,
    models: ranking.models.map((model) => ({ ...model, cost: modelCost(run, model.modelId) }))
  };
}

function percentile(sorted: number[], fraction: number): number {
  if (sorted.length === 0) return 0;
  return sorted[Math.min(sorted.length - 1, Math.floor((sorted.length - 1) * fraction))];
}

function selectedModels(item: ReviewItem): { accepted: Set<string>; preferred: Set<string> } {
  const accepted = new Set<string>();
  const preferred = new Set<string>();
  if (!item.vote || item.vote.customName) return { accepted, preferred };
  for (const option of item.options) {
    if (option.label === item.vote.preferredLabel) {
      option.modelIds.forEach((modelId) => {
        accepted.add(modelId);
        preferred.add(modelId);
      });
    }
    if (item.vote.acceptableLabels.includes(option.label)) {
      option.modelIds.forEach((modelId) => accepted.add(modelId));
    }
  }
  return { accepted, preferred };
}

export function rankRun(run: EvaluationRun): RankingReport {
  if (run.reviewItems.some((item) => !item.vote)) throw new Error("Every review vote must be submitted first.");
  const pairedFunctions = new Set(run.reviewItems.map((item) => item.functionId));
  const eligibleFunctionIds = run.manifest.files.flatMap((file) => file.randomizedFunctionIds);
  const selections = new Map(run.reviewItems.map((item) => [item.key, selectedModels(item)]));
  const models: ModelRanking[] = [];

  for (const modelId of run.manifest.modelIds) {
    let accepted = 0;
    let preferred = 0;
    const perFile = new Map<string, { accepted: number; total: number }>();
    for (const item of run.reviewItems) {
      const selection = selections.get(item.key)!;
      if (selection.accepted.has(modelId)) accepted++;
      if (selection.preferred.has(modelId)) preferred++;
      const file = perFile.get(item.filePath) ?? { accepted: 0, total: 0 };
      file.total++;
      if (selection.accepted.has(modelId)) file.accepted++;
      perFile.set(item.filePath, file);
    }
    const requests = eligibleFunctionIds.map((functionId) => run.requests[requestKey(functionId, modelId)]);
    const successes = requests.filter((request) => request?.status === "success");
    const attempts = requests.flatMap((request) => request?.attempts ?? []);
    const latencies = attempts.map((attempt) => attempt.latencyMs).sort((left, right) => left - right);
    const providers: Record<string, number> = {};
    let prompt = 0;
    let completion = 0;
    let total = 0;
    for (const attempt of attempts) {
      if (attempt.provider) providers[attempt.provider] = (providers[attempt.provider] ?? 0) + 1;
      prompt += attempt.promptTokens ?? 0;
      completion += attempt.completionTokens ?? 0;
      total += attempt.totalTokens ?? 0;
    }
    const perFileAcceptanceRate = Object.fromEntries(
      [...perFile].map(([file, counts]) => [file, counts.total ? counts.accepted / counts.total : 0])
    );
    const fileRates = Object.values(perFileAcceptanceRate);
    const declarationCount = run.reviewItems.length;
    const completionRate = requests.length ? successes.length / requests.length : 0;
    models.push({
      modelId,
      accepted,
      preferred,
      declarationCount,
      acceptanceRate: declarationCount ? accepted / declarationCount : 0,
      preferredRate: declarationCount ? preferred / declarationCount : 0,
      completionRate,
      qualifies: completionRate >= 0.9,
      validationFailures: attempts.filter(
        (attempt) => attempt.failureKind === "schema" || attempt.failureKind === "validation"
      ).length,
      terminalFailures: requests.filter((request) => request?.status === "failed").length,
      retryCount: requests.filter((request) => (request?.attempts.length ?? 0) > 1).length,
      latencyMs: {
        mean: latencies.length ? latencies.reduce((sum, value) => sum + value, 0) / latencies.length : 0,
        p50: percentile(latencies, 0.5),
        p95: percentile(latencies, 0.95)
      },
      providers,
      tokens: { prompt, completion, total },
      cost: modelCost(run, modelId),
      perFileAcceptanceRate,
      equalWeightPerFileRate: fileRates.length
        ? fileRates.reduce((sum, value) => sum + value, 0) / fileRates.length
        : 0
    });
  }

  models.sort(
    (left, right) =>
      right.acceptanceRate - left.acceptanceRate ||
      right.preferredRate - left.preferredRate ||
      left.modelId.localeCompare(right.modelId)
  );
  return {
    generatedAt: new Date().toISOString(),
    pairedFunctionCount: pairedFunctions.size,
    excludedPairedFunctionCount: eligibleFunctionIds.length - pairedFunctions.size,
    models
  };
}
