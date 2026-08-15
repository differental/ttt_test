#!/usr/bin/env python3
# ttt_test - Speed test for random games of tic tac toe
#
# This is free and unencumbered software released into the public domain.
# For more information, please refer to <https://unlicense.org/>
"""Render benchmark JSON from ci/bench.py into the README's marked sections.

Idempotent: rendering the same results twice produces no diff.
"""

import argparse
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
README_PATH = os.path.join(ROOT, "README.md")

#: platform.machine() -> the marker name used in README.md
SECTIONS = {"x86_64": "x64", "aarch64": "arm64"}


def marker_span(text: str, section: str) -> tuple[int, int]:
    start = f"<!-- BENCH:{section}:START -->"
    end = f"<!-- BENCH:{section}:END -->"
    start_at = text.find(start)
    end_at = text.find(end)
    if start_at < 0 or end_at < 0:
        sys.exit(f"README.md is missing the {start} / {end} markers")
    if end_at < start_at:
        sys.exit(f"README.md has {end} before {start}")
    return start_at + len(start), end_at


def fmt_ms(value: float) -> str:
    return f"{value:.0f}"


def render(payload: dict) -> str:
    results = payload["results"]

    rows = []
    for entry in results.values():
        if not entry.get("in_bench"):
            continue
        if entry.get("status") != "ok":
            rows.append((float("inf"), entry["label"], "n/a"))
            continue
        rows.append((entry["best_ms"], entry["label"], fmt_ms(entry["best_ms"])))
    rows.sort(key=lambda row: (row[0], row[1]))

    repeat = payload["repeat"]
    lines = [
        "",
        f"| Language | Best of {repeat} (ms) |",
        "| -------- | ------------------- |",
    ]
    lines += [f"| {label} | {shown} |" for _, label, shown in rows]
    lines.append("")

    extras = [
        entry for entry in results.values()
        if not entry.get("in_bench") and entry.get("status") == "ok"
    ]
    for entry in sorted(extras, key=lambda e: e["label"]):
        note = f"{entry['label']} — {fmt_ms(entry['best_ms'])} ms"
        if entry.get("threaded"):
            note += f" on {payload['cores']} cores"
        lines.append(f"- Not comparable, verified but excluded: {note}")
    if extras:
        lines.append("")

    lines += [
        f"Measured on `{payload['runner']}` ({payload['cpu']}, "
        f"{payload['cores']} cores), Linux {payload['kernel']}, "
        f"commit `{payload['commit']}`, {payload['date']}.",
        "",
        f"Best of {repeat} runs"
        + (", pinned to a single core" if payload.get("pinned") else "")
        + ". Shared CI runners are noisy: treat differences under about 20% as",
        "indistinguishable, and compare languages within a table rather than across",
        "tables or against the reference machine above.",
        "",
    ]
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results", nargs="+", required=True,
                        help="one or more results JSON files from ci/bench.py")
    parser.add_argument("--dry-run", action="store_true",
                        help="print the rendered sections instead of writing")
    args = parser.parse_args()

    with open(README_PATH) as handle:
        text = handle.read()

    for path in args.results:
        with open(path) as handle:
            payload = json.load(handle)
        arch = payload["arch"]
        section = SECTIONS.get(arch)
        if section is None:
            sys.exit(f"{path}: no README section for arch {arch!r}")

        block = render(payload)
        if args.dry_run:
            print(f"=== BENCH:{section} ===")
            print(block)
            continue
        start, end = marker_span(text, section)
        text = text[:start] + block + text[end:]

    if args.dry_run:
        return 0

    with open(README_PATH) as handle:
        if handle.read() == text:
            print("README.md already up to date")
            return 0

    with open(README_PATH, "w") as handle:
        handle.write(text)
    print(f"updated {README_PATH}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
