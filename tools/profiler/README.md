# Tracy profile comparison

Requires Python 3 and Tracy's `tracy-csvexport.exe`. The script defaults to the
bundled exporter in `profiler/windows-0.12.2/`; use `--exporter` for another version.
Run from the repository root:

```powershell
python tools/profiler/compare_profiles.py profiler/traces/domain_rune_2.tracy profiler/traces/domain_rune_3.tracy --start 4.1
```

Compare self times, select zones, and save the full table:

```powershell
python tools/profiler/compare_profiles.py profiler/traces/domain_rune_2.tracy profiler/traces/domain_rune_3.tracy --start 4.1 --self --filter Instance --csv out/profiler/instance_comparison.csv
```

- Times are **median milliseconds per CPU zone invocation**, with event counts for
  each capture. Negative changes mean faster. Self time subtracts measured child
  zones; it still includes uninstrumented callees and waits.
- `--start` and `--end` select events by their start timestamp relative to the
  trace's time origin. The default includes the entire capture; use a comparable
  gameplay interval to exclude startup/loading. The end boundary is exclusive.
- `--top` controls the console row limit (default 30). CSV output includes all rows.
- Zones match by name and source file, allowing line numbers to shift after edits.
  Multiple same-name zones in one file match in source-line order. Adding/removing
  such zones can make that matching ambiguous; inspect the reported line numbers.
- Thread events are pooled per source location. Tracy's exporter does not expose
  per-event overridden zone names, so manager/actor names set with `ZONE_NAME`
  appear under their static names, such as `Level_Manage` or `Manage`.
- Medians of command-processing batches can change when the worker wakes more
  often. Compare event counts too; a lower batch median alone does not establish
  less work per frame. GPU timestamps are not included.
