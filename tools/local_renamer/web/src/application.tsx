import React, { useEffect, useRef, useState } from "react";

type ApplicationRun = { runId: string; status: "ready" | "running" | "review" | "applied" | "failed"; error?: string; active: boolean; modelIds: string[]; files: Array<{ relativePath: string; functionCount: number; eligibleFunctionCount: number }>; requestStats: { pending: number; success: number; failed: number }; reviewCount: number; submittedChoiceCount: number; totalItemCount: number; appliedItemCount: number; selectedItemCount: number };
type ApplicationItem = { key: string; filePath: string; functionName: string; declarationId: string; oldName: string; functionSource: string; highlightRanges: Array<{ start: number; end: number }>; options: Array<{ decision: string; modelIds: string[]; confidences: number[]; warnings: string[] }>; failures: Array<{ modelId: string; error: string }>; choice?: { decision: string; customName?: string; warnings?: string[] } };

async function api<T>(url: string, options?: RequestInit): Promise<T> {
  const response = await fetch(url, { ...options, headers: { "Content-Type": "application/json", ...(options?.headers ?? {}) } });
  const body = await response.json();
  if (!response.ok) throw new Error(body.error ?? `HTTP ${response.status}`);
  return body as T;
}
function sourceFilePaths(files: string): string[] { return files.split(/\r?\n/).map((entry) => entry.trim()).filter(Boolean); }
const activeRunStorageKey = "kya-local-renamer.application-run.v1";

function RunReview({ run, refresh }: { run: ApplicationRun; refresh: () => Promise<void> }) {
  const [items, setItems] = useState<ApplicationItem[]>([]);
  const [selectedKey, setSelectedKey] = useState("");
  const [selectedDecision, setSelectedDecision] = useState("");
  const [customName, setCustomName] = useState("");
  const [error, setError] = useState("");
  const [notice, setNotice] = useState("");
  const highlightedName = useRef<HTMLElement>(null);
  async function load(advance = false) {
    const result = await api<{ items: ApplicationItem[] }>(`/api/application/runs/${run.runId}/review`);
    setItems(result.items);
    setSelectedKey((current) => {
      const currentItem = result.items.find((candidate) => candidate.key === current);
      return advance || !currentItem ? result.items.find((candidate) => !candidate.choice)?.key ?? currentItem?.key ?? "" : current;
    });
  }
  useEffect(() => { void load(); }, [run.runId, run.appliedItemCount]);
  const item = items.find((candidate) => candidate.key === selectedKey);
  useEffect(() => { highlightedName.current?.scrollIntoView({ block: "center", inline: "nearest" }); }, [item?.key]);
  useEffect(() => { setSelectedDecision(item?.choice?.customName ? "" : item?.choice?.decision ?? ""); setCustomName(item?.choice?.customName ?? ""); setError(""); setNotice(""); }, [item?.key]);
  function sourceWithHighlights(reviewItem: ApplicationItem): React.ReactNode[] {
    const nodes: React.ReactNode[] = []; let cursor = 0;
    for (const [index, range] of reviewItem.highlightRanges.entries()) { nodes.push(reviewItem.functionSource.slice(cursor, range.start)); nodes.push(<mark ref={index === 0 ? highlightedName : undefined} key={`${range.start}-${range.end}`}>{reviewItem.functionSource.slice(range.start, range.end)}</mark>); cursor = range.end; }
    nodes.push(reviewItem.functionSource.slice(cursor)); return nodes;
  }
  async function saveChoice(decision = selectedDecision, advance = true, name = customName) {
    if (!item) return;
    setError(""); setNotice("");
    try {
      await api(`/api/application/runs/${run.runId}/review/${item.key}`, { method: "POST", body: JSON.stringify(name.trim() ? { customName: name.trim() } : { decision }) });
      await load(advance); await refresh();
      setNotice("Selection saved. Moved to the next undecided name.");
    } catch (reason) { setError(reason instanceof Error ? reason.message : String(reason)); }
  }
  if (!item) return <section className="review-ledger review-loading" aria-busy="true"><nav className="review-nav" aria-label="Rename ledger"><p className="eyebrow">Rename ledger</p></nav><div className="panel review-detail"><p className="notice">Loading names for review…</p></div></section>;
  const suggestions = [{ decision: item.oldName, modelIds: [] as string[], confidences: [] as number[], warnings: [] as string[] }, ...item.options.filter((option) => option.decision !== item.oldName)];
  const groups = [...items.reduce((result, candidate) => { const group = result.get(candidate.functionName) ?? []; group.push(candidate); result.set(candidate.functionName, group); return result; }, new Map<string, ApplicationItem[]>()).entries()];
  return <section className="review-ledger" aria-label="Rename review ledger">
    <nav className="review-nav" aria-label="Declarations to review"><div className="review-nav-heading"><p className="eyebrow">Rename ledger</p><strong>{run.selectedItemCount}/{run.totalItemCount} selected</strong></div>
      {groups.map(([functionName, functionItems]) => <section className="ledger-function" key={functionName}><div><strong>{functionName}</strong><small>{functionItems.filter((candidate) => candidate.choice).length}/{functionItems.length} selected</small></div>
        {functionItems.map((candidate) => <button className={`ledger-row${candidate.key === item.key ? " active" : ""}${candidate.choice ? " complete" : ""}`} aria-current={candidate.key === item.key ? "true" : undefined} key={candidate.key} onClick={() => setSelectedKey(candidate.key)}><code>{candidate.oldName}</code><span>{candidate.choice?.decision ?? "Needs a name"}</span></button>)}</section>)}</nav>
    <div className="panel review-detail"><div className="section-heading"><div><p className="eyebrow">Review {items.findIndex((candidate) => candidate.key === item.key) + 1} of {items.length}</p><h2>{item.functionName}</h2><p className="muted">{item.filePath} · {item.declarationId}</p></div><span className="count">{item.choice ? "Selected" : "Needs decision"}</span></div>
      {error && <p className="error" role="alert">{error}</p>}{notice && <p className="notice" aria-live="polite">{notice}</p>}
      <p className="current-name">Original name <code>{item.oldName}</code></p><pre className="source-pane" aria-label={`Source for ${item.functionName}; ${item.oldName} is highlighted`}><code>{sourceWithHighlights(item)}</code></pre>
      <fieldset className="answers"><legend>Choose a name for <code>{item.oldName}</code></legend>{suggestions.map((option) => { const selected = !customName.trim() && selectedDecision === option.decision; const modelCount = option.modelIds.length; const confidence = modelCount ? option.confidences.reduce((sum, value) => sum + value, 0) / modelCount : 0; const confidenceTier = confidence >= 0.85 ? "strong" : confidence >= 0.65 ? "medium" : modelCount ? "tentative" : "neutral"; return <label className={`answer choice confidence-${confidenceTier}${selected ? " selected" : ""}`} key={option.decision}><input type="radio" name={`choice-${item.key}`} checked={selected} onChange={() => { setSelectedDecision(option.decision); setCustomName(""); void saveChoice(option.decision, true, ""); }} /><span><strong>{option.decision === item.oldName ? "Keep existing name" : option.decision}</strong>{modelCount > 0 && <small>Suggested by {modelCount} configured {modelCount === 1 ? "model" : "models"}</small>}</span>{modelCount > 0 && <div className="evidence"><small>Models: {option.modelIds.join(", ")}</small><small>{confidenceTier} confidence · {option.confidences.map((value) => value.toFixed(2)).join(", ")}</small></div>}{option.warnings.length > 0 && <small className="warning">Valid, but {option.warnings.join(", ")}</small>}</label>; })}</fieldset>
      {item.failures.length > 0 && <details className="model-failures"><summary>{item.failures.length} model {item.failures.length === 1 ? "response failed" : "responses failed"}</summary>{item.failures.map((failure) => <p className="error" role="alert" key={failure.modelId}><code>{failure.modelId}</code>: {failure.error}</p>)}</details>}
      <label>Or enter a custom name<input value={customName} onChange={(event) => { setCustomName(event.target.value); setSelectedDecision(""); }} aria-describedby="custom-name-help" /><small id="custom-name-help">The name is validated before it can be saved.</small></label>
      <div className="actions review-actions"><button className="primary" disabled={!customName.trim()} onClick={() => void saveChoice()}>Save custom name and next undecided</button></div><p className="muted selection-help">Saved selections remain editable until you apply their function.</p>
    </div>
  </section>;
}

export function ApplicationMode() {
  const [run, setRun] = useState<ApplicationRun | null>(null); const [files, setFiles] = useState("src/b-witch/ActorShip.cpp"); const [configuredModels, setConfiguredModels] = useState<string[]>([]); const [error, setError] = useState("");
  useEffect(() => { void api<{ modelIds: string[] }>("/api/application/config").then((config) => setConfiguredModels(config.modelIds)).catch((reason) => setError(String(reason))); const runId = window.localStorage.getItem(activeRunStorageKey); if (runId) void api<ApplicationRun>(`/api/application/runs/${encodeURIComponent(runId)}`).then(setRun).catch(() => window.localStorage.removeItem(activeRunStorageKey)); }, []);
  async function refresh() { if (!run) return; try { setRun(await api<ApplicationRun>(`/api/application/runs/${run.runId}`)); } catch (reason) { setError(String(reason)); } }
  useEffect(() => { if (!run || (!run.active && run.status !== "running")) return; const timer = setInterval(() => void refresh(), 1200); return () => clearInterval(timer); }, [run?.runId, run?.status, run?.active]);
  async function createRun() { setError(""); try { const created = await api<ApplicationRun>("/api/application/runs", { method: "POST", body: JSON.stringify({ files: sourceFilePaths(files) }) }); window.localStorage.setItem(activeRunStorageKey, created.runId); setRun(created); } catch (reason) { setError(reason instanceof Error ? reason.message : String(reason)); } }
  async function start() { if (!run) return; setError(""); try { await api(`/api/application/runs/${run.runId}/start`, { method: "POST" }); setRun({ ...run, status: "running", active: true }); } catch (reason) { setError(reason instanceof Error ? reason.message : String(reason)); } }
  function startNewRun() { window.localStorage.removeItem(activeRunStorageKey); setError(""); setRun(null); }

  const stage = run?.status === "ready" ? "Ready to generate" : run?.status === "running" ? "Suggestions generating" : run?.status === "review" ? "Name locals" : run?.status === "applied" ? "Changes applied" : "Generation failed";
  return <main><header><div className="section-heading"><div><p className="eyebrow">Kya tooling</p><h1>Local renamer</h1><p>Generate suggestions and choose names. Each function is applied automatically as soon as all of its locals are named.</p></div>{run && <button onClick={startNewRun}>New rename run</button>}</div></header>{error && <p className="error" role="alert">{error}</p>}
    {!run ? <section className="panel"><div className="section-heading"><div><p className="eyebrow">Run mode</p><h2>Source files</h2></div></div><p className="muted">Configured models: {configuredModels.join(", ") || "Loading..."}</p><label>Repository-relative C++ source files, one per line<textarea rows={6} value={files} onChange={(event) => setFiles(event.target.value)} /></label><button className="primary" disabled={sourceFilePaths(files).length === 0 || configuredModels.length === 0} onClick={createRun}>Create rename run</button></section> : <><section className="panel status"><div className="section-heading"><div><p className="eyebrow">Rename run {run.runId}</p><h2>{stage}</h2></div><span className="count">{run.selectedItemCount}/{run.totalItemCount} selected</span></div><div className="status-metrics"><span>{run.appliedItemCount} applied</span><span>{run.totalItemCount - run.appliedItemCount} still being named</span><span>{run.requestStats.success} model responses</span><span>{run.requestStats.failed} failed</span><span>{run.requestStats.pending} pending</span></div>{run.error && <p className="error" role="alert">{run.error}</p>}{(run.status === "ready" || (run.status === "running" && !run.active)) && <button className="primary" onClick={start}>{run.status === "running" ? "Resume generation" : "Generate suggestions"}</button>}{run.status === "applied" && <p className="notice">All completed functions were applied automatically.</p>}{!run.active && ["review", "applied", "failed"].includes(run.status) && <button onClick={startNewRun}>New rename run</button>}</section>{run.status === "review" && <RunReview run={run} refresh={refresh} />}</>}
  </main>;
}
