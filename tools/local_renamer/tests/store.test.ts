import { randomUUID } from "node:crypto";
import { rm } from "node:fs/promises";
import path from "node:path";
import { afterEach, describe, expect, it } from "vitest";
import { RunStore } from "../src/store.js";

const temporaryRoots: string[] = [];

afterEach(async () => {
  await Promise.all(temporaryRoots.splice(0).map((root) => rm(root, { recursive: true, force: true })));
});

describe("local model availability cache", () => {
  it("survives store recreation and replaces models that fail a later test", async () => {
    const root = path.join(process.cwd(), "evaluation", `test-cache-${randomUUID()}`);
    temporaryRoots.push(root);
    const checkedAt = new Date().toISOString();
    await new RunStore(root).replaceAvailableModels({
      "model/one": { checkedAt },
      "model/two": { checkedAt }
    });

    const restartedStore = new RunStore(root);
    expect(Object.keys(await restartedStore.getAvailableModels())).toEqual(["model/one", "model/two"]);

    await restartedStore.replaceAvailableModels({ "model/two": { checkedAt } });
    expect(await restartedStore.getAvailableModels()).toEqual({ "model/two": { checkedAt } });

    await restartedStore.updateAvailableModels({
      "model/paid": { checkedAt },
      "model/missing": null
    });
    expect(await restartedStore.getAvailableModels()).toEqual({
      "model/two": { checkedAt },
      "model/paid": { checkedAt }
    });
  });
});
