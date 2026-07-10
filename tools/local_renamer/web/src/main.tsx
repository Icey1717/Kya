import React, { useEffect, useMemo, useRef, useState } from "react";

import { createRoot } from "react-dom/client";
import "./styles.css";
import { ApplicationMode } from "./application";

type Model = {
  id: string;
  name: string;
  contextLength: number;
  promptPrice: number;
  completionPrice: number;
  paid: boolean;
  supportedParameters: string[];
  rateLimitedUntil?: string;
  rateLimitNote?: string;
  availability: "untested" | "available" | "unavailable";
  availabilityNote?: string;
  availabilityCheckedAt?: string;
};
type Vote = { preferredLabel?: string; acceptableLabels: string[]; customName?: string };
type ReviewItem = {
  key: string;
  filePath: string;
  functionName: string;
  declarationId: string;
  oldName: string;
  functionSource: string;
  highlightRanges: Array<{ start: number; end: number }>;
  options: Array<{ label: string; decision: string; confidences?: number[] }>;
  vote?: Vote;
};
type Run = {
  runId: string;
  status: string;
  active: boolean;
  updatedAt: string;
  error?: string;
  files: Array<{ relativePath: string; functionCount: number; eligibleFunctionCount: number; reviewAvailable: boolean }>;
  modelIds: string[];
  primaryRequestCount: number;
  worstCaseRequestCount: number;
  completedRequests: number;
  requestStats: { pending: number; success: number; failed: number; attempts: number };
  recentActivity: Array<{
    modelId: string;
    filePath: string;
    functionId: string;
    attempt: number;
    status: "success" | "failure";
    finishedAt: string;
    latencyMs: number;
    failureKind?: string;
    error?: string;
  }>;
  reviewCount: number;
  submittedVoteCount: number;
  ranking?: {
    pairedFunctionCount: number;
    excludedPairedFunctionCount: number;
    models: Array<{
      modelId: string;
      acceptanceRate: number;
      preferredRate: number;
      completionRate: number;
      qualifies: boolean;
      validationFailures: number;
      latencyMs: { mean: number };
      cost: { total: number; perSuccessfulAnswer: number; estimated: boolean };
    }>;
  };
  retainedModelIds?: string[];
};

async function api<T>(url: string, options?: RequestInit): Promise<T> {
  const response = await fetch(url, {
    ...options,
    headers: { "Content-Type": "application/json", ...(options?.headers ?? {}) }
  });
  const body = await response.json();
  if (!response.ok) throw new Error(body.error ?? `HTTP ${response.status}`);
  return body as T;
}

const setupStorageKey = "kya-local-renamer.setup.v1";
const activeRunStorageKey = "kya-local-renamer.active-run.v1";

interface SetupState {
  selectedModelIds: string[];
  files: string;
  timeoutSeconds: number;
}

const defaultSetupState: SetupState = {
  selectedModelIds: [],
  files: "src/b-witch/ActorShip.cpp",
  timeoutSeconds: 120
};

function sourceFilePaths(files: string): string[] {
  return files.split(/\r?\n/).map((entry) => entry.trim()).filter(Boolean);
}

function loadSetupState(): SetupState {
  if (typeof window === "undefined") return defaultSetupState;
  try {
    const parsed = JSON.parse(window.localStorage.getItem(setupStorageKey) ?? "null") as Partial<SetupState> | null;
    if (!parsed) return defaultSetupState;
    const timeoutSeconds = typeof parsed.timeoutSeconds === "number" && Number.isFinite(parsed.timeoutSeconds) && parsed.timeoutSeconds > 0
      ? parsed.timeoutSeconds
      : defaultSetupState.timeoutSeconds;
    return {
      selectedModelIds: Array.isArray(parsed.selectedModelIds) ? parsed.selectedModelIds.filter((value): value is string => typeof value === "string") : [],
      files: typeof parsed.files === "string" ? parsed.files : defaultSetupState.files,
      timeoutSeconds
    };
  } catch {
    return defaultSetupState;
  }
}

function saveSetupState(state: SetupState): void {
  if (typeof window === "undefined") return;
  window.localStorage.setItem(setupStorageKey, JSON.stringify(state));
}
function Setup({ onSelected }: { onSelected: (run: Run) => void }) {
  const initialSetup = loadSetupState();
  const [models, setModels] = useState<Model[]>([]);
  const [selected, setSelected] = useState<Set<string>>(() => new Set(initialSetup.selectedModelIds));
  const [files, setFiles] = useState(() => initialSetup.files);
  const [timeoutSeconds, setTimeoutSeconds] = useState(() => initialSetup.timeoutSeconds);
  const [error, setError] = useState("");
  const [loading, setLoading] = useState(true);
  const [testing, setTesting] = useState<"free" | "paid" | "">("");
  const [resumeRunId, setResumeRunId] = useState("");

  useEffect(() => {
    let cancelled = false;
    async function loadModels() {
      try {
        const result = await api<{ models: Model[] }>("/api/models");
        if (!cancelled) setModels(result.models);
      } catch (reason) {
        if (!cancelled) setError(String(reason));
      } finally {
        if (!cancelled) setLoading(false);
      }
    }
    void loadModels();
    const interval = setInterval(() => void loadModels(), 30_000);
    return () => {
      cancelled = true;
      clearInterval(interval);
    };
  }, []);

  useEffect(() => {
    if (loading) return;
    const modelsById = new Map(models.map((model) => [model.id, model]));
    setSelected((current) => new Set([...current].filter((id) => {
      const model = modelsById.get(id);
      return Boolean(model && model.availability !== "unavailable" && !model.rateLimitedUntil);
    })));
  }, [loading, models]);

  useEffect(() => {
    saveSetupState({
      selectedModelIds: [...selected],
      files,
      timeoutSeconds
    });
  }, [selected, files, timeoutSeconds]);

  const filePaths = sourceFilePaths(files);

  async function createRun() {
    setError("");
    try {
      const run = await api<Run>("/api/evaluation/runs", {
        method: "POST",
        body: JSON.stringify({
          modelIds: [...selected],
          files: filePaths,
          timeoutMs: timeoutSeconds * 1000
        })
      });
      onSelected(run);
      saveSetupState({
        selectedModelIds: [...selected],
        files,
        timeoutSeconds
      });
    } catch (reason) {
      setError(reason instanceof Error ? reason.message : String(reason));
    }
  }

  async function resumeRun() {
    setError("");
    try {
      const run = await api<Run>(`/api/evaluation/runs/${encodeURIComponent(resumeRunId.trim())}`);
      onSelected(run);
    } catch (reason) {
      setError(reason instanceof Error ? reason.message : String(reason));
    }
  }

  async function testModels(tier: "free" | "paid") {
    setError("");
    setTesting(tier);
    try {
      const result = await api<{ models: Model[] }>("/api/models/test", {
        method: "POST",
        body: JSON.stringify({ tier })
      });
      setModels(result.models);
    } catch (reason) {
      setError(reason instanceof Error ? reason.message : String(reason));
    } finally {
      setTesting("");
    }
  }

  const selectedModelsVerified = [...selected].every((id) => {
    const model = models.find((candidate) => candidate.id === id);
    return model?.availability === "available" && !model.rateLimitedUntil;
  });
  const freeModels = models.filter((model) => !model.paid);
  const paidModels = models.filter((model) => model.paid);

  function modelList(sectionModels: Model[]) {
    return (
      <div className="model-list" aria-busy={loading}>
        {sectionModels.map((model) => {
          const unavailable = model.availability !== "available" || Boolean(model.rateLimitedUntil);
          const disabled = unavailable && !selected.has(model.id);
          return (
            <label className={`model${unavailable ? " disabled" : ""}`} key={model.id}>
              <input
                type="checkbox"
                checked={selected.has(model.id)}
                disabled={disabled}
                onChange={() => {
                  const next = new Set(selected);
                  next.has(model.id) ? next.delete(model.id) : next.add(model.id);
                  setSelected(next);
                }}
              />
              <span>
                <strong>{model.id}</strong>
                <small>
                  {model.contextLength.toLocaleString()} context
                  {model.supportedParameters.includes("structured_outputs") ? " · JSON Schema" : " · strict JSON"}
                  {model.paid && ` · $${(model.promptPrice * 1_000_000).toFixed(3)}/$${(model.completionPrice * 1_000_000).toFixed(3)} per M input/output tokens`}
                </small>
                {model.rateLimitedUntil && <small className="muted">Temporarily rate limited until {new Date(model.rateLimitedUntil).toLocaleTimeString()}</small>}
                {!model.rateLimitedUntil && model.availability === "untested" && <small className="muted">Not tested</small>}
                {!model.rateLimitedUntil && model.availability === "available" && <small className="available">Available</small>}
                {!model.rateLimitedUntil && model.availability === "unavailable" && <small className="unavailable">Unavailable: {model.availabilityNote ?? "no provider responded"}</small>}
              </span>
            </label>
          );
        })}
      </div>
    );
  }

  return (
    <>
    <section className="panel">
      <div className="section-heading">
        <div><p className="eyebrow">Existing evaluation</p><h2>Resume a frozen run</h2></div>
      </div>
      <label>
        Run ID
        <input value={resumeRunId} onChange={(event) => setResumeRunId(event.target.value)} placeholder="20260628013038-972aaba4" />
      </label>
      <button disabled={!resumeRunId.trim()} onClick={resumeRun}>Resume frozen run</button>
    </section>
    <section className="panel">
      <div className="section-heading">
        <div>
          <p className="eyebrow">New evaluation</p>
          <h2>Freeze the candidate set and source files</h2>
        </div>
        <span className="count">{selected.size} selected</span>
      </div>
      {error && <p className="error">{error}</p>}
      <label>
        Repository-relative source files, one per line ({filePaths.length} {filePaths.length === 1 ? "file" : "files"})
        <textarea rows={5} value={files} onChange={(event) => setFiles(event.target.value)} />
      </label>
      <label>
        Request timeout in seconds
        <input type="number" min={1} value={timeoutSeconds} onChange={(event) => setTimeoutSeconds(Number(event.target.value))} />
      </label>
      <div className="model-section-heading">
        <div><p className="eyebrow">Free models</p><h3>Zero-priced candidates</h3></div>
        <div className="actions">
          <button disabled={loading || Boolean(testing) || freeModels.length === 0} onClick={() => testModels("free")}>
            {testing === "free" ? "Re-testing free models…" : "Re-test free model availability"}
          </button>
          <span className="muted model-test-summary">
            {freeModels.filter((model) => model.availability === "available" && !model.rateLimitedUntil).length} of {freeModels.length} verified
          </span>
        </div>
      </div>
      {modelList(freeModels)}
      <div className="model-section-heading paid-heading">
        <div><p className="eyebrow">Paid models</p><h3>Curated paid candidates</h3></div>
        <div className="actions">
          <button disabled={loading || Boolean(testing) || paidModels.length === 0} onClick={() => testModels("paid")}>
            {testing === "paid" ? "Re-testing paid models…" : "Re-test paid model availability"}
          </button>
          <span className="muted model-test-summary">
            {paidModels.filter((model) => model.availability === "available" && !model.rateLimitedUntil).length} of {paidModels.length} verified
          </span>
        </div>
      </div>
      <p className="paid-warning">The paid availability test and evaluation requests use account credits at the catalog prices shown.</p>
      {modelList(paidModels)}
      <button className="primary" disabled={selected.size < 2 || filePaths.length === 0 || !selectedModelsVerified || loading || Boolean(testing)} onClick={createRun}>
        Create frozen run
      </button>
    </section>
    </>
  );
}

function Review({ run, refresh }: { run: Run; refresh: () => Promise<void> }) {
  const [items, setItems] = useState<ReviewItem[]>([]);
  const [selectedKey, setSelectedKey] = useState("");
  const [preferred, setPreferred] = useState("");
  const [acceptable, setAcceptable] = useState<Set<string>>(new Set());
  const [customName, setCustomName] = useState("");
  const [error, setError] = useState("");
  const highlightedName = useRef<HTMLElement>(null);

  async function load(advance = false) {
    const result = await api<{ items: ReviewItem[] }>(`/api/evaluation/runs/${run.runId}/review`);
    setItems(result.items);
    setSelectedKey((current) => {
      const currentItem = result.items.find((candidate) => candidate.key === current);
      if (advance || !currentItem || currentItem.vote) {
        return result.items.find((candidate) => !candidate.vote)?.key ?? currentItem?.key ?? result.items[0]?.key ?? "";
      }
      return current;
    });
  }
  useEffect(() => {
    void load();
  }, [run.runId]);
  const item = items.find((candidate) => candidate.key === selectedKey);
  useEffect(() => {
    highlightedName.current?.scrollIntoView({ block: "center", inline: "nearest" });
  }, [item?.key]);

  function sourceWithHighlights(reviewItem: ReviewItem): React.ReactNode[] {
    const nodes: React.ReactNode[] = [];
    let cursor = 0;
    for (const [index, range] of reviewItem.highlightRanges.entries()) {
      nodes.push(reviewItem.functionSource.slice(cursor, range.start));
      nodes.push(
        <mark ref={index === 0 ? highlightedName : undefined} key={`${range.start}-${range.end}`}>
          {reviewItem.functionSource.slice(range.start, range.end)}
        </mark>
      );
      cursor = range.end;
    }
    nodes.push(reviewItem.functionSource.slice(cursor));
    return nodes;
  }

  async function vote() {
    if (!item) return;
    setError("");
    try {
      await api(`/api/evaluation/runs/${run.runId}/review/${item.key}/vote`, {
        method: "POST",
        body: JSON.stringify(
          customName
            ? { customName }
            : { preferredLabel: preferred, acceptableLabels: [...acceptable].filter((label) => label !== preferred) }
        )
      });
      setPreferred("");
      setAcceptable(new Set());
      setCustomName("");
      await load(true);
      await refresh();
    } catch (reason) {
      setError(reason instanceof Error ? reason.message : String(reason));
    }
  }

  if (!item) return <p>No paired declarations are available for review.</p>;
  return (
    <section className="panel">
      <div className="section-heading">
        <div>
          <p className="eyebrow">Blind review</p>
          <h2>{item.functionName}</h2>
          <p className="muted">{item.filePath} · {item.declarationId}</p>
        </div>
        <span className="count">{run.submittedVoteCount}/{run.reviewCount}</span>
      </div>
      {error && <p className="error">{error}</p>}
      <p className="current-name">Current name <code>{item.oldName}</code></p>
      <pre className="source-pane" aria-label={`Source for ${item.functionName}`}><code>{sourceWithHighlights(item)}</code></pre>
      <div className="answers">
        {item.options.map((option) => (
          <div className="answer" key={option.label}>
            <label>
              <input
                type="radio"
                name="preferred"
                disabled={Boolean(item.vote)}
                checked={preferred === option.label || item.vote?.preferredLabel === option.label}
                onChange={() => { setPreferred(option.label); setCustomName(""); }}
              />
              <span><small>{option.label}</small><strong>{option.decision === "keep" ? "Keep existing" : option.decision}</strong></span>
            </label>
            <label className="acceptable">
              <input
                type="checkbox"
                disabled={Boolean(item.vote)}
                checked={acceptable.has(option.label) || Boolean(item.vote?.acceptableLabels.includes(option.label))}
                onChange={() => {
                  const next = new Set(acceptable);
                  next.has(option.label) ? next.delete(option.label) : next.add(option.label);
                  setAcceptable(next);
                }}
              />
              also acceptable
            </label>
            {option.confidences && <small className="confidence">confidence {option.confidences.map((value) => value.toFixed(2)).join(", ")}</small>}
          </div>
        ))}
      </div>
      <label>
        Or enter a different preferred name
        <input disabled={Boolean(item.vote)} value={item.vote?.customName ?? customName} onChange={(event) => { setCustomName(event.target.value); setPreferred(""); setAcceptable(new Set()); }} />
      </label>
      {!item.vote
        ? <button className="primary" onClick={vote}>Submit and lock vote</button>
        : <p className="notice">All declarations have been reviewed. Finalize the run above to reveal the ranking.</p>}
    </section>
  );
}

function Results({ run, refresh }: { run: Run; refresh: () => Promise<void> }) {
  const [retained, setRetained] = useState(new Set(run.retainedModelIds ?? []));
  const [exportPath, setExportPath] = useState("");
  if (!run.ranking) return null;
  async function saveRetention() {
    await api(`/api/evaluation/runs/${run.runId}/retained-models`, {
      method: "PUT",
      body: JSON.stringify({ modelIds: [...retained] })
    });
    await refresh();
  }
  async function exportRun() {
    const result = await api<{ path: string }>(`/api/evaluation/runs/${run.runId}/export`, { method: "POST" });
    setExportPath(result.path);
  }
  return (
    <section className="panel">
      <div className="section-heading"><div><p className="eyebrow">Final ranking</p><h2>Measured results</h2></div></div>
      <table>
        <thead><tr><th>Retain</th><th>Model</th><th>Acceptance</th><th>Preferred</th><th>Completion</th><th>Cost / answer</th><th>Total cost</th><th>Mean latency</th><th>Valid</th></tr></thead>
        <tbody>
          {run.ranking.models.map((model) => (
            <tr key={model.modelId}>
              <td><input type="checkbox" checked={retained.has(model.modelId)} onChange={() => { const next = new Set(retained); next.has(model.modelId) ? next.delete(model.modelId) : next.add(model.modelId); setRetained(next); }} /></td>
              <td><code>{model.modelId}</code></td>
              <td>{(model.acceptanceRate * 100).toFixed(1)}%</td>
              <td>{(model.preferredRate * 100).toFixed(1)}%</td>
              <td>{(model.completionRate * 100).toFixed(1)}%</td>
              <td>{model.cost.estimated ? "≈" : ""}${model.cost.perSuccessfulAnswer.toFixed(6)}</td>
              <td>{model.cost.estimated ? "≈" : ""}${model.cost.total.toFixed(6)}</td>
              <td>{Math.round(model.latencyMs.mean)} ms</td>
              <td>{model.qualifies ? "yes" : "no"}</td>
            </tr>
          ))}
        </tbody>
      </table>
      <div className="actions"><button onClick={saveRetention}>Save retained set</button><button onClick={exportRun}>Export reproducibility bundle</button></div>
      {exportPath && <p className="muted">Exported to <code>{exportPath}</code></p>}
    </section>
  );
}

function EvaluationApp() {
  const [run, setRun] = useState<Run | null>(null);
  const [restoringRun, setRestoringRun] = useState(true);
  const [error, setError] = useState("");
  const [actionPending, setActionPending] = useState<"start" | "pause" | "finalize" | "">("");
  const [notice, setNotice] = useState("");
  function selectRun(selectedRun: Run) {
    window.localStorage.setItem(activeRunStorageKey, selectedRun.runId);
    setRun(selectedRun);
  }
  useEffect(() => {
    const runId = window.localStorage.getItem(activeRunStorageKey);
    if (!runId) {
      setRestoringRun(false);
      return;
    }
    void api<Run>(`/api/evaluation/runs/${encodeURIComponent(runId)}`)
      .then(setRun)
      .catch(() => window.localStorage.removeItem(activeRunStorageKey))
      .finally(() => setRestoringRun(false));
  }, []);
  async function refresh() {
    if (!run) return;
    try {
      setRun(await api<Run>(`/api/evaluation/runs/${run.runId}`));
    } catch (reason) {
      setError(reason instanceof Error ? reason.message : String(reason));
    }
  }
  useEffect(() => {
    if (!run || (!run.active && !["running", "paused"].includes(run.status))) return;
    const timer = setInterval(() => void refresh(), 1500);
    return () => clearInterval(timer);
  }, [run?.runId, run?.status]);
  const progress = useMemo(() => run ? Math.round((run.completedRequests / Math.max(1, run.primaryRequestCount)) * 100) : 0, [run]);

  async function action(name: "start" | "pause" | "finalize") {
    if (!run) return;
    setError("");
    setNotice("");
    setActionPending(name);
    try {
      await api(`/api/evaluation/runs/${run.runId}/${name}`, { method: "POST" });
      if (name === "start") {
        // Persisting the running state is asynchronous. Start polling from the
        // acknowledgement instead of immediately reloading a stale ready run.
        setRun((current) => current ? { ...current, status: "running", active: true } : current);
        setNotice("Start accepted. Waiting for the first request batch to finish.");
      } else {
        setNotice(name === "pause" ? "Pause requested; active requests will finish first." : "Run finalized.");
        await refresh();
      }
    } catch (reason) {
      setError(reason instanceof Error ? reason.message : String(reason));
    } finally {
      setActionPending("");
    }
  }

  return (
    <main>
      <header><p className="eyebrow">Kya tooling</p><h1>Model evaluator</h1><p>Blinded local-variable naming evaluation. Source files remain read-only.</p></header>
      {error && <p className="error">{error}</p>}
      {restoringRun ? <p className="notice">Restoring the last frozen run…</p> : !run ? <Setup onSelected={selectRun} /> : (
        <>
          <section className="panel status">
            <div className="section-heading"><div><p className="eyebrow">Run {run.runId}</p><h2>{run.status}</h2></div><span className="count">{progress}%</span></div>
            <progress max={run.primaryRequestCount} value={run.completedRequests} />
            <p>{run.completedRequests} / {run.primaryRequestCount} primary request outcomes · worst case {run.worstCaseRequestCount}</p>
            <div className="run-feedback" aria-live="polite">
              <strong>{run.active ? "Worker active" : "Worker idle"}</strong>
              <span>{run.requestStats.success} succeeded · {run.requestStats.failed} failed · {run.requestStats.pending} pending · {run.requestStats.attempts} attempts</span>
              <small>Last persisted update {new Date(run.updatedAt).toLocaleString()}</small>
            </div>
            {notice && <p className="notice">{notice}</p>}
            {run.error && <p className="error">{run.error}</p>}
            <div className="actions">
              {(["ready", "paused", "failed"].includes(run.status) || (run.status === "running" && !run.active)) && <button className="primary" disabled={Boolean(actionPending)} onClick={() => action("start")}>{actionPending === "start" ? "Starting…" : run.status === "running" ? "Recover / resume" : "Start / resume"}</button>}
              {run.status === "running" && run.active && <button disabled={Boolean(actionPending)} onClick={() => action("pause")}>{actionPending === "pause" ? "Requesting pause…" : "Pause after active requests"}</button>}
              {run.status === "review" && run.submittedVoteCount === run.reviewCount && <button className="primary" disabled={Boolean(actionPending)} onClick={() => action("finalize")}>{actionPending === "finalize" ? "Finalizing…" : "Finalize and reveal ranking"}</button>}
            </div>
          </section>
          {run.recentActivity.length > 0 && (
            <section className="panel activity">
              <div className="section-heading"><div><p className="eyebrow">Request log</p><h2>Recent outcomes</h2></div></div>
              <ol className="activity-list">
                {run.recentActivity.map((entry, index) => (
                  <li key={`${entry.modelId}-${entry.functionId}-${entry.attempt}-${entry.finishedAt}-${index}`}>
                    <span className={`outcome ${entry.status}`}>{entry.status}</span>
                    <code>{entry.modelId}</code>
                    <span>{entry.filePath} · {entry.functionId} · attempt {entry.attempt} · {(entry.latencyMs / 1000).toFixed(1)}s</span>
                    {entry.error && <small>{entry.failureKind}: {entry.error}</small>}
                  </li>
                ))}
              </ol>
            </section>
          )}
          {run.status === "review" && <Review run={run} refresh={refresh} />}
          {run.status === "finalized" && <Results run={run} refresh={refresh} />}
        </>
      )}
    </main>
  );
}

function App() {
  const [mode, setMode] = useState<"evaluate" | "run">(() => {
    return window.localStorage.getItem("kya-local-renamer.mode") === "run" ? "run" : "evaluate";
  });
  function selectMode(next: "evaluate" | "run") {
    window.localStorage.setItem("kya-local-renamer.mode", next);
    setMode(next);
  }
  return (
    <>
      <nav className="mode-switch" aria-label="Tool mode">
        <button className={mode === "evaluate" ? "active" : ""} onClick={() => selectMode("evaluate")}>Evaluate models</button>
        <button className={mode === "run" ? "active" : ""} onClick={() => selectMode("run")}>Run renamer</button>
      </nav>
      {mode === "evaluate" ? <EvaluationApp /> : <ApplicationMode />}
    </>
  );
}

createRoot(document.getElementById("root")!).render(<React.StrictMode><App /></React.StrictMode>);
