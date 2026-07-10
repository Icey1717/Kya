import React, { useEffect, useRef, useState } from "react";

type ApplicationRun = {
  runId: string;
  status: "ready" | "running" | "review" | "applied" | "failed";
  error?: string;
  active: boolean;
  modelIds: string[];
  files: Array<{ relativePath: string; functionCount: number; eligibleFunctionCount: number }>;
  requestStats: { pending: number; success: number; failed: number };
  reviewCount: number;
  submittedChoiceCount: number;
  readyFunctions: Array<{ functionId: string; functionName: string }>;
};

type ApplicationItem = {
  key: string;
  filePath: string;
  functionName: string;
  declarationId: string;
  oldName: string;
  functionSource: string;
  highlightRanges: Array<{ start: number; end: number }>;
  options: Array<{ decision: string; modelIds: string[]; confidences: number[]; warnings: string[] }>;
  failures: Array<{ modelId: string; error: string }>;
  choice?: { decision: string; customName?: string; warnings?: string[] };
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

function sourceFilePaths(files: string): string[] {
  return files.split(/\r?\n/).map((entry) => entry.trim()).filter(Boolean);
}

const activeRunStorageKey = "kya-local-renamer.application-run.v1";

function RunReview({ run, refresh }: { run: ApplicationRun; refresh: () => Promise<void> }) {
  const [items, setItems] = useState<ApplicationItem[]>([]);
  const [selectedKey, setSelectedKey] = useState("");
  const [customName, setCustomName] = useState("");
  const [error, setError] = useState("");
  const highlightedName = useRef<HTMLElement>(null);

  async function load(advance = false) {
    const result = await api<{ items: ApplicationItem[] }>(`/api/application/runs/${run.runId}/review`);
    setItems(result.items);
    setSelectedKey((current) => {
      const currentItem = result.items.find((candidate) => candidate.key === current);
      if (advance || !currentItem || currentItem.choice) return result.items.find((candidate) => !candidate.choice)?.key ?? currentItem?.key ?? "";
      return current;
    });
  }
  useEffect(() => { void load(); }, [run.runId]);
  const item = items.find((candidate) => candidate.key === selectedKey);
  useEffect(() => { highlightedName.current?.scrollIntoView({ block: "center", inline: "nearest" }); }, [item?.key]);

  function sourceWithHighlights(reviewItem: ApplicationItem): React.ReactNode[] {
    const nodes: React.ReactNode[] = [];
    let cursor = 0;
    for (const [index, range] of reviewItem.highlightRanges.entries()) {
      nodes.push(reviewItem.functionSource.slice(cursor, range.start));
      nodes.push(<mark ref={index === 0 ? highlightedName : undefined} key={`${range.start}-${range.end}`}>{reviewItem.functionSource.slice(range.start, range.end)}</mark>);
      cursor = range.end;
    }
    nodes.push(reviewItem.functionSource.slice(cursor));
    return nodes;
  }

  async function choose(decision?: string) {
    if (!item) return;
    setError("");
    try {
      await api(`/api/application/runs/${run.runId}/review/${item.key}`, {
        method: "POST",
        body: JSON.stringify(customName.trim() ? { customName: customName.trim() } : { decision })
      });
      setCustomName("");
      await load(true);
      await refresh();
    } catch (reason) {
      setError(reason instanceof Error ? reason.message : String(reason));
    }
  }

  if (!item) return <p className="notice">No eligible local declarations were found in these files.</p>;
  return (
    <section className="panel">
      <div className="section-heading">
        <div><p className="eyebrow">Run review</p><h2>{item.functionName}</h2><p className="muted">{item.filePath} · {item.declarationId}</p></div>
        <span className="count">{run.submittedChoiceCount}/{run.reviewCount}</span>
      </div>
      {error && <p className="error">{error}</p>}
      <p className="current-name">Current name <code>{item.oldName}</code></p>
      <pre className="source-pane"><code>{sourceWithHighlights(item)}</code></pre>
      {!item.choice && <button onClick={() => choose(item.oldName)}>Use existing name</button>}
      <div className="answers">
        {item.options.map((option) => (
          <button className="answer choice" key={option.decision} disabled={Boolean(item.choice)} onClick={() => choose(option.decision)}>
            <span><strong>{option.decision === item.oldName ? "Keep existing" : option.decision}</strong><small>{option.modelIds.join(", ")}</small></span>
            <small>confidence {option.confidences.map((value) => value.toFixed(2)).join(", ")}</small>
            {option.warnings.length > 0 && <small className="warning">warning: {option.warnings.join(", ")}</small>}
          </button>
        ))}
      </div>
      {item.failures.length > 0 && <div className="model-failures">{item.failures.map((failure) => <p className="error" key={failure.modelId}><code>{failure.modelId}</code>: {failure.error}</p>)}</div>}
      <label>
        Custom name
        <input disabled={Boolean(item.choice)} value={item.choice?.customName ?? customName} onChange={(event) => setCustomName(event.target.value)} />
      </label>
      {item.choice?.warnings && item.choice.warnings.length > 0 && <p className="warning">warning: {item.choice.warnings.join(", ")}</p>}
      {!item.choice && <button disabled={!customName.trim()} onClick={() => choose()}>Use custom name</button>}
    </section>
  );
}

export function ApplicationMode() {
  const [run, setRun] = useState<ApplicationRun | null>(null);
  const [files, setFiles] = useState("src/b-witch/ActorShip.cpp");
  const [configuredModels, setConfiguredModels] = useState<string[]>([]);
  const [error, setError] = useState("");
  const [applyingFunction, setApplyingFunction] = useState("");

  useEffect(() => {
    void api<{ modelIds: string[] }>("/api/application/config").then((config) => setConfiguredModels(config.modelIds)).catch((reason) => setError(String(reason)));
    const runId = window.localStorage.getItem(activeRunStorageKey);
    if (runId) void api<ApplicationRun>(`/api/application/runs/${encodeURIComponent(runId)}`).then(setRun).catch(() => window.localStorage.removeItem(activeRunStorageKey));
  }, []);

  async function refresh() {
    if (!run) return;
    try { setRun(await api<ApplicationRun>(`/api/application/runs/${run.runId}`)); } catch (reason) { setError(String(reason)); }
  }
  useEffect(() => {
    if (!run || (!run.active && run.status !== "running")) return;
    const timer = setInterval(() => void refresh(), 1200);
    return () => clearInterval(timer);
  }, [run?.runId, run?.status, run?.active]);

  async function createRun() {
    setError("");
    try {
      const created = await api<ApplicationRun>("/api/application/runs", { method: "POST", body: JSON.stringify({ files: sourceFilePaths(files) }) });
      window.localStorage.setItem(activeRunStorageKey, created.runId);
      setRun(created);
    } catch (reason) { setError(reason instanceof Error ? reason.message : String(reason)); }
  }
  async function start() {
    if (!run) return;
    setError("");
    try {
      await api(`/api/application/runs/${run.runId}/start`, { method: "POST" });
      setRun({ ...run, status: "running", active: true });
    } catch (reason) { setError(reason instanceof Error ? reason.message : String(reason)); }
  }
  function startNewRun() {
    window.localStorage.removeItem(activeRunStorageKey);
    setError("");
    setRun(null);
  }
  async function applyFunction(functionId: string) {
    if (!run) return;
    setApplyingFunction(functionId);
    setError("");
    try { setRun(await api<ApplicationRun>(`/api/application/runs/${run.runId}/functions/${functionId}/apply`, { method: "POST" })); }
    catch (reason) { setError(reason instanceof Error ? reason.message : String(reason)); }
    finally { setApplyingFunction(""); }
  }

  return (
    <main>
      <header><div className="section-heading"><div><p className="eyebrow">Kya tooling</p><h1>Local renamer</h1><p>Choose a configured model suggestion or a validated custom name, then apply each completed function.</p></div>{run && <button onClick={startNewRun}>New rename run</button>}</div></header>
      {error && <p className="error">{error}</p>}
      {!run ? (
        <section className="panel">
          <div className="section-heading"><div><p className="eyebrow">Run mode</p><h2>Source files</h2></div></div>
          <p className="muted">Configured models: {configuredModels.join(", ") || "Loading..."}</p>
          <label>Repository-relative C++ source files, one per line<textarea rows={6} value={files} onChange={(event) => setFiles(event.target.value)} /></label>
          <button className="primary" disabled={sourceFilePaths(files).length === 0 || configuredModels.length === 0} onClick={createRun}>Create rename run</button>
        </section>
      ) : (
        <>
          <section className="panel status">
            <div className="section-heading"><div><p className="eyebrow">Rename run {run.runId}</p><h2>{run.status}</h2></div><span className="count">{run.submittedChoiceCount}/{run.reviewCount}</span></div>
            <p>{run.requestStats.success} model responses · {run.requestStats.failed} failed · {run.requestStats.pending} pending</p>
            {run.error && <p className="error">{run.error}</p>}
            {(run.status === "ready" || (run.status === "running" && !run.active)) && <button className="primary" onClick={start}>{run.status === "running" ? "Resume generation" : "Generate suggestions"}</button>}
            {run.status === "review" && run.readyFunctions.map((functionInfo) => <button className="primary" disabled={Boolean(applyingFunction)} key={functionInfo.functionId} onClick={() => applyFunction(functionInfo.functionId)}>{applyingFunction === functionInfo.functionId ? "Applying..." : `Apply ${functionInfo.functionName}`}</button>)}
            {run.status === "applied" && <p className="notice">Approved names were applied to every reviewed function.</p>}
            {!run.active && ["review", "applied", "failed"].includes(run.status) && <button onClick={startNewRun}>New rename run</button>}
          </section>
          {run.status === "review" && <RunReview run={run} refresh={refresh} />}
        </>
      )}
    </main>
  );
}
