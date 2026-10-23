import type { Finding } from "./functionAudit.js";

interface ReportRow {
  name: string;
  path: string;
  line: number;
  status: string;
  address?: string;
  findings: Finding[];
  reviewedFindings: { finding: Finding; reason: string }[];
  error?: string;
}

function markdown(value: string) { return value.replace(/[|\r\n]/g, " "); }

export function renderAuditReports(results: ReportRow[], counts: Record<string, number>) {
  function table(rows: ReportRow[]) {
    return ["| Function | Source | PS2 address | Status | Signals |", "|---|---|---|---|---|", ...rows.map(row => {
      const signals = row.findings.length ? row.findings.map(finding => finding.detail).join(" ")
        : row.reviewedFindings.map(review => `Reviewed: ${review.reason}`).join(" ");
      return `| ${markdown(row.name)} | ${markdown(row.path)}:${row.line} | ${row.address ?? ""} | ${row.status} | ${markdown(signals || row.error || "")} |`;
    })];
  }
  const candidates = results.filter(row => row.status === "candidate");
  const coverage = results.filter(row => !["candidate", "no_findings", "reviewed"].includes(row.status));
  const reviewed = results.filter(row => row.reviewedFindings.length);
  const overview = `Coverage: ${JSON.stringify(counts)}`;
  return {
    report: ["# Function audit", "", "Read-only heuristic triage. A candidate is not a confirmed bug; no_findings does not establish equivalence.", "", overview, "",
      `${candidates.length} candidates with unresolved signals. Expected differences with matching reviewed evidence are omitted from this queue.`, "",
      "See [coverage gaps](coverage.md) for functions that could not be compared and [reviewed differences](reviewed.md) for suppressed findings. report.json retains every result, normalization and reviewed finding.", "",
      ...table(candidates)].join("\n") + "\n",
    coverage: ["# Function audit coverage gaps", "", overview, "", "These functions have incomplete coverage; they are not cleared by the candidate filters. See [candidate queue](report.md).", "", ...table(coverage)].join("\n") + "\n",
    reviewed: ["# Reviewed function audit differences", "", "Only exact findings covered by matching source, PS2 and dependency hashes are suppressed. A function can still have unresolved findings in the [candidate queue](report.md).", "",
      ...table(reviewed.map(row => ({ ...row, findings: [] })))].join("\n") + "\n"
  };
}
