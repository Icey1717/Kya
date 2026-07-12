import { randomUUID } from "node:crypto";
import { mkdir, readFile, rm, writeFile } from "node:fs/promises";
import path from "node:path";
import { afterEach, describe, expect, it } from "vitest";
import {
  applyApplicationFunction,
  applyApplicationRun,
  createApplicationRun,
  publicApplicationItems,
  submitApplicationChoice
} from "../src/application.js";
import type { ModelMetadata } from "../src/types.js";

const roots: string[] = [];

const model: ModelMetadata = {
  id: "test/model:free",
  name: "test",
  contextLength: 32768,
  promptPrice: 0,
  completionPrice: 0,
  inputModalities: ["text"],
  outputModalities: ["text"],
  supportedParameters: [],
  raw: {}
};

async function fixture(source: string) {
  const root = path.join(process.cwd(), "evaluation", `application-${randomUUID()}`);
  roots.push(root);
  const repositoryRoot = path.join(root, "repo");
  await mkdir(path.join(repositoryRoot, "src"), { recursive: true });
  const file = path.join(repositoryRoot, "src", "fixture.cpp");
  await writeFile(file, source, "utf8");
  const run = await createApplicationRun({
    repositoryRoot,
    relativePaths: ["src/fixture.cpp"],
    models: [model],
    timeoutMs: 5000
  });
  run.status = "review";
  return { file, run };
}

afterEach(async () => {
  await Promise.all(roots.splice(0).map((root) => rm(root, { recursive: true, force: true })));
});

describe("application run mutation", () => {
  it("always accepts an explicit choice to use the existing name", async () => {
    const { run } = await fixture("int test() { int iVar1 = 1; return iVar1; }\n");
    submitApplicationChoice(run, run.items[0].key, { decision: "iVar1" });
    expect(run.items[0].choice?.decision).toBe("iVar1");
  });
  it("skips a completed function when every reviewed name stays unchanged", async () => {
    const { file, run } = await fixture("int test() { int iVar1 = 1; return iVar1; }\n");
    submitApplicationChoice(run, run.items[0].key, { decision: "iVar1" });

    expect(run.items[0].appliedAt).toBeDefined();
    expect(run.status).toBe("applied");
    expect(await readFile(file, "utf8")).toBe("int test() { int iVar1 = 1; return iVar1; }\n");
  });

  it("maps a model keep decision to the declaration's existing name", async () => {
    const { run } = await fixture("int test() { int iVar1 = 1; return iVar1; }\n");
    const request = Object.values(run.requests)[0];
    request.status = "success";
    request.normalized = {
      suggestions: [{
        declaration_id: "local_001",
        old_name: "iVar1",
        suggested_name: "iVar1",
        confidence: 0.8,
        decision: "keep",
        keep: true
      }]
    };

    expect(publicApplicationItems(run)[0].options[0].decision).toBe("iVar1");
    submitApplicationChoice(run, run.items[0].key, { decision: "iVar1" });
    expect(run.items[0].choice?.decision).toBe("iVar1");
  });
  it("replaces every bound occurrence after a reviewed custom choice", async () => {
    const { file, run } = await fixture("int test() { int iVar1 = 1; return iVar1; }\n");
    const item = run.items[0];
    submitApplicationChoice(run, item.key, { customName: "value" });

    await applyApplicationRun(run);

    expect(run.status).toBe("applied");
    expect(await readFile(file, "utf8")).toBe("int test() { int value = 1; return value; }\n");
  });

  it("applies completed functions independently and rebases later source offsets", async () => {
    const { file, run } = await fixture([
      "int first() { int iVar1 = 1; return iVar1; }",
      "int second() { int iVar2 = 2; return iVar2; }",
      ""
    ].join("\n"));
    const first = run.files[0].functions.find((fn) => fn.qualifiedName === "first")!;
    const second = run.files[0].functions.find((fn) => fn.qualifiedName === "second")!;
    submitApplicationChoice(run, run.items.find((item) => item.functionId === first.id)!.key, { customName: "firstValue" });

    await applyApplicationFunction(run, first.id);

    expect(await readFile(file, "utf8")).toContain("int firstValue = 1; return firstValue;");
    submitApplicationChoice(run, run.items.find((item) => item.functionId === second.id)!.key, { customName: "secondValue" });
    await applyApplicationFunction(run, second.id);

    expect(run.status).toBe("applied");
    expect(await readFile(file, "utf8")).toContain("int secondValue = 2; return secondValue;");
  });
  it("refuses to write when a selected source changed after the run was created", async () => {
    const { file, run } = await fixture("int test() { int iVar1 = 1; return iVar1; }\n");
    submitApplicationChoice(run, run.items[0].key, { customName: "value" });
    await writeFile(file, "int test() { int iVar1 = 2; return iVar1; }\n", "utf8");

    await expect(applyApplicationRun(run)).rejects.toThrow(/changed since run creation/i);
    expect(await readFile(file, "utf8")).toContain("iVar1 = 2");
  });

  it("rejects a reviewed name that collides with another local", async () => {
    const { run } = await fixture("int test() { int iVar1 = 1; int value = 2; return iVar1 + value; }\n");
    await expect(Promise.resolve().then(() => submitApplicationChoice(run, run.items[0].key, { customName: "value" })))
      .rejects.toThrow(/collision/i);
  });
});
