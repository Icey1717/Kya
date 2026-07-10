export type RequestFailureKind =
  | "transport"
  | "timeout"
  | "rate_limit"
  | "schema"
  | "validation"
  | "http"
  | "paused";

export interface ModelMetadata {
  id: string;
  name: string;
  contextLength: number;
  promptPrice: number;
  completionPrice: number;
  inputModalities: string[];
  outputModalities: string[];
  supportedParameters: string[];
  raw: unknown;
}

export interface ModelRateLimitStatus {
  blockedUntil: string;
  lastFailureAt: string;
  retryAfterMs: number;
  failureCount: number;
  lastError?: string;
}

export interface ModelAvailabilityCacheEntry {
  checkedAt: string;
}

export interface Declaration {
  id: string;
  oldName: string;
  declaredType: string;
  startIndex: number;
  endIndex: number;
  occurrences?: Array<{ startIndex: number; endIndex: number }>;
  scopePath: number[];
  isPointer: boolean;
  isBoolean: boolean;
}

export interface FunctionRecord {
  id: string;
  qualifiedName: string;
  relativePath: string;
  startIndex: number;
  endIndex: number;
  startLine: number;
  endLine: number;
  source: string;
  promptSource?: string;
  parameters: string[];
  declarations: Declaration[];
  parseError: boolean;
  exclusionReason?: "no_locals" | "ambiguous_binding" | "parse_failure" | "context_limit";
  estimatedPromptTokens?: number;
}

export interface FileRecord {
  relativePath: string;
  sha256: string;
  functions: FunctionRecord[];
  randomizedFunctionIds: string[];
  reviewAvailable: boolean;
}

export interface Suggestion {
  declaration_id: string;
  old_name: string;
  suggested_name: string;
  confidence: number;
}

export interface NormalizedSuggestion extends Suggestion {
  decision: string;
  keep: boolean;
  warnings?: string[];
}

export interface NormalizedResponse {
  suggestions: NormalizedSuggestion[];
}

export interface AttemptRecord {
  attempt: 1 | 2;
  startedAt: string;
  finishedAt: string;
  latencyMs: number;
  status: "success" | "failure";
  failureKind?: RequestFailureKind;
  error?: string;
  provider?: string;
  promptTokens?: number;
  completionTokens?: number;
  totalTokens?: number;
  cost?: number;
  rawResponseFile?: string;
}

export interface RequestRecord {
  modelId: string;
  functionId: string;
  filePath: string;
  status: "pending" | "success" | "failed";
  attempts: AttemptRecord[];
  normalized?: NormalizedResponse;
}

export interface EvaluationConfig {
  timeoutMs: number;
  temperature: 0;
  globalConcurrency: 3;
  maxPerModel: 1;
}

export interface ReviewOptionInternal {
  label: string;
  decision: string;
  modelIds: string[];
  confidences: number[];
}

export interface ReviewItem {
  key: string;
  filePath: string;
  functionId: string;
  functionName: string;
  declarationId: string;
  oldName: string;
  options: ReviewOptionInternal[];
  vote?: ReviewVote;
}

export interface ReviewVote {
  preferredLabel?: string;
  acceptableLabels: string[];
  customName?: string;
  submittedAt: string;
}

export interface ModelRanking {
  modelId: string;
  accepted: number;
  preferred: number;
  declarationCount: number;
  acceptanceRate: number;
  preferredRate: number;
  completionRate: number;
  qualifies: boolean;
  validationFailures: number;
  terminalFailures: number;
  retryCount: number;
  latencyMs: { mean: number; p50: number; p95: number };
  providers: Record<string, number>;
  tokens: { prompt: number; completion: number; total: number };
  cost: { total: number; perSuccessfulAnswer: number; estimated: boolean };
  perFileAcceptanceRate: Record<string, number>;
  equalWeightPerFileRate: number;
}

export interface RankingReport {
  generatedAt: string;
  pairedFunctionCount: number;
  excludedPairedFunctionCount: number;
  models: ModelRanking[];
}

export interface RunManifest {
  schemaVersion: 1;
  runId: string;
  createdAt: string;
  repositoryRoot: string;
  requestedFileCount: number;
  modelIds: string[];
  models: ModelMetadata[];
  files: FileRecord[];
  config: EvaluationConfig;
  primaryRequestCount: number;
  worstCaseRequestCount: number;
}

export interface EvaluationRun {
  manifest: RunManifest;
  status: "ready" | "running" | "paused" | "review" | "finalized" | "failed";
  requests: Record<string, RequestRecord>;
  reviewItems: ReviewItem[];
  ranking?: RankingReport;
  retainedModelIds: string[];
  error?: string;
  updatedAt: string;
}

export interface PublicReviewOption {
  label: string;
  decision: string;
  confidences?: number[];
}

export interface PublicReviewItem {
  key: string;
  filePath: string;
  functionId: string;
  functionName: string;
  declarationId: string;
  oldName: string;
  functionSource: string;
  highlightRanges: Array<{ start: number; end: number }>;
  options: PublicReviewOption[];
  vote?: ReviewVote;
}
