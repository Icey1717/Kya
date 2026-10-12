#!/usr/bin/env python3
"""Compare median CPU zone times from two Tracy captures using tracy-csvexport."""

import argparse
import csv
import math
from pathlib import Path
import statistics
import subprocess
import sys
import tempfile


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_EXPORTER = REPO_ROOT / "profiler/windows-0.12.2/tracy-csvexport.exe"


def read_profile(profile, exporter, *, start=0.0, end=None, self_time=False, zone_filter=""):
    command = [str(exporter), "-u", "-s", "\t"]
    if self_time:
        command.append("-e")
    if zone_filter:
        command.extend(["-f", zone_filter])
    command.append(str(profile))
    zones = {}
    # Stream stdout rather than retaining millions of CSV dictionaries in memory.
    # Stderr goes to a file so exporter diagnostics cannot fill a pipe and deadlock.
    with tempfile.TemporaryFile(mode="w+t", encoding="utf-8") as errors:
        with subprocess.Popen(command, stdout=subprocess.PIPE, stderr=errors,
                              text=True, encoding="utf-8", errors="replace") as process:
            try:
                reader = csv.DictReader(process.stdout, delimiter="\t")
                required = {"name", "src_file", "src_line", "ns_since_start", "exec_time_ns"}
                if not required.issubset(reader.fieldnames or []):
                    raise ValueError("Exporter did not return the expected CPU event columns")
                for row in reader:
                    timestamp = int(row["ns_since_start"]) / 1e9
                    if timestamp < start or (end is not None and timestamp >= end):
                        continue
                    source = row["src_file"].replace("\\", "/")
                    key = (row["name"], source, int(row["src_line"]))
                    zones.setdefault(key, []).append(int(row["exec_time_ns"]))
            except BaseException:
                process.kill()
                process.wait()
                raise
            if process.wait() != 0:
                errors.seek(0)
                raise ValueError(f"Exporter failed for {profile}: {errors.read().strip()}")
    return zones


def summarize(zones):
    """Match unique name/file pairs regardless of source-line changes."""
    grouped = {}
    for (name, source, line), times in zones.items():
        grouped.setdefault((name, source), []).append((line, times))
    result = {}
    for (name, source), locations in grouped.items():
        # Repeated names in one file remain separate, in source order.
        for ordinal, (line, times) in enumerate(sorted(locations)):
            result[(name, source, ordinal)] = {
                "line": line, "count": len(times),
                "median_ms": statistics.median(times) / 1e6,
            }
    return result


def compare(before, after):
    rows = []
    for key in before.keys() | after.keys():
        old, new = before.get(key), after.get(key)
        old_ms = old["median_ms"] if old else None
        new_ms = new["median_ms"] if new else None
        delta = new_ms - old_ms if old and new else None
        change = delta / old_ms * 100 if old and new and old_ms != 0 else None
        rows.append({
            "name": key[0], "source": key[1], "occurrence": key[2] + 1,
            "before_line": old["line"] if old else None,
            "after_line": new["line"] if new else None,
            "before_count": old["count"] if old else 0,
            "after_count": new["count"] if new else 0,
            "before_median_ms": old_ms, "after_median_ms": new_ms,
            "delta_ms": delta, "change_percent": change,
            "status": "matched" if old and new else "removed" if old else "added",
        })
    return sorted(rows, key=lambda r: (
        -max(r["before_median_ms"] or 0, r["after_median_ms"] or 0),
        r["name"], r["source"], r["occurrence"]))


def positive_integer(value):
    number = int(value)
    if number < 1:
        raise argparse.ArgumentTypeError("must be at least 1")
    return number


def nonnegative_seconds(value):
    number = float(value)
    if not math.isfinite(number) or number < 0:
        raise argparse.ArgumentTypeError("must be a finite, nonnegative number")
    return number


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("before", type=Path)
    parser.add_argument("after", type=Path)
    parser.add_argument("--exporter", type=Path, default=DEFAULT_EXPORTER)
    parser.add_argument("--start", type=nonnegative_seconds, default=0,
                        help="include events starting at or after this trace timestamp, in seconds")
    parser.add_argument("--end", type=nonnegative_seconds,
                        help="exclude events starting at or after this trace timestamp, in seconds")
    parser.add_argument("--self", action="store_true", dest="self_time",
                        help="subtract measured child-zone time")
    parser.add_argument("--filter", default="", help="case-insensitive zone-name substring")
    parser.add_argument("--top", type=positive_integer, default=30, help="console row limit (default: 30)")
    parser.add_argument("--csv", type=Path, help="write all comparison rows to this CSV file")
    args = parser.parse_args()
    if args.end is not None and args.end <= args.start:
        parser.error("--end must be greater than --start")
    for path in (args.before, args.after, args.exporter):
        if not path.is_file():
            parser.error(f"file not found: {path}")
    try:
        profiles = []
        for profile in (args.before, args.after):
            print(f"Reading {profile}...", file=sys.stderr, flush=True)
            profiles.append(summarize(read_profile(
                profile.resolve(), args.exporter.resolve(), start=args.start, end=args.end,
                self_time=args.self_time, zone_filter=args.filter)))
        rows = compare(*profiles)
    except (OSError, ValueError, csv.Error) as error:
        parser.exit(1, f"Error: {error}\n")
    if not rows:
        parser.exit(1, "No CPU zones matched the selected range/filter.\n")
    if args.csv:
        args.csv.parent.mkdir(parents=True, exist_ok=True)
        with args.csv.open("w", newline="", encoding="utf-8") as output:
            writer = csv.DictWriter(output, fieldnames=list(rows[0]))
            writer.writeheader()
            writer.writerows(rows)
    print(f"Before: {args.before.name} | After: {args.after.name}")
    print(f"{'Self' if args.self_time else 'Inclusive'} CPU medians; event starts in "
          f"[{args.start:g}, {args.end if args.end is not None else 'end'}) seconds")
    print("Negative delta/change means faster. Times are per invocation, not per frame.")
    visible_rows = rows[:args.top]
    labels = [f"{row['name']} [{Path(row['source']).name}:{row['before_line'] or '-'}->{row['after_line'] or '-'}]"
              for row in visible_rows]
    width = max(len("Zone"), *(len(label) for label in labels))
    print(f"{'Zone':{width}} {'Before ms':>10} {'After ms':>10} {'Delta ms':>10} {'Change':>9} {'Counts before/after':>22}")
    for row, label in zip(visible_rows, labels):
        def number(value, signed=False):
            return "-" if value is None else format(value, "+.4f" if signed else ".4f")
        change = "-" if row["change_percent"] is None else f"{row['change_percent']:+.1f}%"
        counts = f"{row['before_count']}/{row['after_count']}"
        print(f"{label:{width}} {number(row['before_median_ms']):>10} {number(row['after_median_ms']):>10} "
              f"{number(row['delta_ms'], True):>10} {change:>9} {counts:>22}")
    print(f"Showing {min(args.top, len(rows))} of {len(rows)} zones, ordered by largest median.")
    if args.csv:
        print(f"Full comparison: {args.csv}")


if __name__ == "__main__":
    main()
