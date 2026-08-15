#!/usr/bin/env python3
# ttt_test - Speed test for random games of tic tac toe
#
# This is free and unencumbered software released into the public domain.
# For more information, please refer to <https://unlicense.org/>
"""Build, verify and time every implementation in this repository.

Two modes:

  --check   build and run each implementation once, then assert it produced the
            right answer (see ci/expected.json).
  --bench   run each implementation --repeat times and report the best result.

Builds always go through `make`, so the Makefile stays the single source of
truth for compiler flags; only the run commands live here.
"""

import argparse
import json
import os
import platform
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass, field
from datetime import datetime, timezone

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXPECTED_PATH = os.path.join(ROOT, "ci", "expected.json")

GAMES = 10000

OUTPUT_RE = re.compile(r"^O/X/Draw: (\d+)/(\d+)/(\d+)$", re.M)
TIME_RE = re.compile(r"^Time taken: ([\d.]+) ms", re.M)

# Installing PyPy through actions/setup-python puts a `python3` on PATH that is
# PyPy, which would silently turn the CPython row into a second PyPy row. CI
# passes both interpreters explicitly so PATH order cannot decide which is which.
CPYTHON = os.environ.get("CPYTHON", "python3")
PYPY = os.environ.get("PYPY", "pypy3")


@dataclass
class Impl:
    name: str
    #: `make` target that builds it, or None when there is nothing to build.
    target: str | None
    #: argv to run, relative to the repository root.
    run: list[str]
    #: README label; {v} is replaced with the detected toolchain version.
    label: str
    #: argv that prints a version, and a regex whose group 1 is the version.
    version_cmd: list[str]
    version_re: str
    #: executables that must exist for this implementation to run at all.
    requires: list[str]
    #: whether it belongs in the README benchmark table.
    in_bench: bool = True
    #: multi-threaded implementations are neither CPU-pinned nor comparable.
    threaded: bool = False
    env: dict[str, str] = field(default_factory=dict)


IMPLS = [
    Impl("rust", "rust", ["./ttt.rs.exe"], "Rust (rustc {v}) -O3",
         ["rustc", "--version"], r"rustc (\S+)", ["rustc"]),
    Impl("cpp", "cpp", ["./ttt.cc.exe"], "C++ (g++ {v}) -O3",
         ["g++", "--version"], r"(\d+\.\d+\.\d+)", ["g++"]),
    Impl("c", "c", ["./ttt.c.exe"], "C (gcc {v}) -O3",
         ["gcc", "--version"], r"(\d+\.\d+\.\d+)", ["gcc"]),
    Impl("go", "go", ["./ttt.go.exe"], "Go {v} (single-threaded)",
         ["go", "version"], r"go(\d+\.\d+(?:\.\d+)?)", ["go"],
         env={"CPUS": "1"}),
    Impl("ocaml", "ocaml", ["./ttt.ml.exe"], "OCaml (ocamlopt flambda {v}) -O3",
         ["ocamlopt", "-version"], r"(\S+)", ["ocamlopt"]),
    Impl("js", None, ["node", "ttt.js"], "JavaScript (node.js {v})",
         ["node", "--version"], r"v?(\S+)", ["node"]),
    Impl("java", "java", ["java", "ttt"], "Java (openjdk {v})",
         ["java", "-version"], r'version "([^"]+)"', ["java", "javac"]),
    Impl("pypy", None, [PYPY, "ttt.py"], "Python (PyPy {v})",
         [PYPY, "--version"], r"PyPy (\S+)", [PYPY]),
    Impl("lean", "lean", ["./ttt.lean.exe"], "Lean {v}",
         ["lean", "--version"], r"version (\d+\.\d+\.\d+)", ["lean"]),
    Impl("python", None, [CPYTHON, "ttt.py"], "Python (CPython {v})",
         [CPYTHON, "--version"], r"Python (\S+)", [CPYTHON]),
    # Not part of the benchmark table. ttt-mt.cc is multi-threaded, and every
    # other entry runs single-threaded; ttt-alt.c plays a different game
    # entirely (40x40 board, win condition 12). Both are still verified.
    Impl("cpp-mt", "cpp-mt", ["./ttt-mt.cc.exe"], "C++ multi-threaded (g++ {v}) -O3",
         ["g++", "--version"], r"(\d+\.\d+\.\d+)", ["g++"],
         in_bench=False, threaded=True),
    Impl("alt-c", "alt", ["./ttt-alt.c.exe"], "C, 40x40 board (gcc {v}) -O3",
         ["gcc", "--version"], r"(\d+\.\d+\.\d+)", ["gcc"], in_bench=False),
]

BY_NAME = {impl.name: impl for impl in IMPLS}


class Failure(Exception):
    pass


def log(msg: str) -> None:
    print(msg, flush=True)


def run(argv, env=None, cwd=ROOT):
    full_env = dict(os.environ)
    full_env.update(env or {})
    return subprocess.run(
        argv, cwd=cwd, env=full_env, capture_output=True, text=True
    )


def detect_version(impl: Impl) -> str:
    """Best-effort toolchain version. Some tools print to stderr (java)."""
    proc = run(impl.version_cmd)
    text = (proc.stdout or "") + (proc.stderr or "")
    match = re.search(impl.version_re, text)
    return match.group(1) if match else "unknown"


def missing_tools(impl: Impl) -> list[str]:
    return [tool for tool in impl.requires if shutil.which(tool) is None]


def build(impl: Impl) -> None:
    if impl.target is None:
        return
    proc = run(["make", impl.target])
    if proc.returncode != 0:
        raise Failure(
            f"make {impl.target} failed (exit {proc.returncode})\n"
            f"{proc.stdout}\n{proc.stderr}"
        )


def pin_prefix(impl: Impl, pin_cpu: bool) -> list[str]:
    """Pin single-threaded runs to one core to cut scheduler migration noise."""
    if not pin_cpu or impl.threaded or shutil.which("taskset") is None:
        return []
    cpu = 2 if (os.cpu_count() or 1) > 2 else (os.cpu_count() or 1) - 1
    return ["taskset", "-c", str(cpu)]


def execute(impl: Impl, pin_cpu: bool) -> tuple[tuple[int, int, int], float]:
    argv = pin_prefix(impl, pin_cpu) + impl.run
    proc = run(argv, env=impl.env)
    if proc.returncode != 0:
        raise Failure(
            f"{impl.name} exited {proc.returncode}\n{proc.stdout}\n{proc.stderr}"
        )

    counts_match = OUTPUT_RE.search(proc.stdout)
    if counts_match is None:
        raise Failure(
            f"{impl.name}: no 'O/X/Draw: a/b/c' line in output:\n{proc.stdout}"
        )
    time_match = TIME_RE.search(proc.stdout)
    if time_match is None:
        raise Failure(
            f"{impl.name}: no 'Time taken: N ms' line in output:\n{proc.stdout}"
        )

    counts = tuple(int(g) for g in counts_match.groups())
    if sum(counts) != GAMES:
        raise Failure(
            f"{impl.name}: counts {counts} sum to {sum(counts)}, expected {GAMES}"
        )
    return counts, float(time_match.group(1))


def load_expected() -> dict:
    with open(EXPECTED_PATH) as handle:
        return json.load(handle)


def cpu_model() -> str:
    try:
        with open("/proc/cpuinfo") as handle:
            for line in handle:
                if line.startswith(("model name", "Model")):
                    return line.split(":", 1)[1].strip()
    except OSError:
        pass
    return platform.processor() or "unknown"


def git_commit() -> str:
    proc = run(["git", "rev-parse", "--short", "HEAD"])
    return proc.stdout.strip() if proc.returncode == 0 else "unknown"


def selected(only: str | None) -> list[Impl]:
    if not only:
        return list(IMPLS)
    names = [name.strip() for name in only.split(",") if name.strip()]
    unknown = [name for name in names if name not in BY_NAME]
    if unknown:
        sys.exit(f"unknown implementation(s): {', '.join(unknown)}")
    return [BY_NAME[name] for name in names]


def check(impls: list[Impl], skip_missing: bool, update_goldens: bool) -> int:
    expected = load_expected()
    goldens = dict(expected["impls"])
    group = expected["identical_group"]

    observed: dict[str, tuple[int, int, int]] = {}
    failures: list[str] = []
    skipped: list[str] = []

    for impl in impls:
        absent = missing_tools(impl)
        if absent:
            message = f"{impl.name}: missing {', '.join(absent)}"
            if skip_missing:
                log(f"SKIP {message}")
                skipped.append(impl.name)
                continue
            failures.append(message)
            continue

        try:
            build(impl)
            counts, elapsed = execute(impl, pin_cpu=False)
        except Failure as err:
            log(f"FAIL {impl.name}")
            failures.append(str(err))
            continue

        observed[impl.name] = counts
        golden = goldens.get(impl.name)
        if update_goldens:
            goldens[impl.name] = list(counts)
        elif golden is None:
            failures.append(f"{impl.name}: no golden in ci/expected.json")
            continue
        elif list(counts) != list(golden):
            failures.append(
                f"{impl.name}: got {'/'.join(map(str, counts))}, "
                f"expected {'/'.join(map(str, golden))}"
            )
            log(f"FAIL {impl.name}")
            continue
        log(f"ok   {impl.name:8s} {'/'.join(map(str, counts))}  {elapsed:g} ms")

    # Cross-language invariant: every implementation of the 20x20 game that
    # seeds the shared xorshift with 1729163 must agree exactly.
    present = [name for name in group if name in observed]
    if len(present) >= 2:
        distinct = {observed[name] for name in present}
        if len(distinct) > 1:
            detail = ", ".join(
                f"{name}={'/'.join(map(str, observed[name]))}" for name in present
            )
            failures.append(f"identical-group disagreement: {detail}")
        else:
            log(f"ok   identical-group agrees across {len(present)}: {', '.join(present)}")

    if update_goldens:
        expected["impls"] = goldens
        with open(EXPECTED_PATH, "w") as handle:
            json.dump(expected, handle, indent=2, sort_keys=True)
            handle.write("\n")
        log(f"wrote {EXPECTED_PATH}")

    if skipped:
        log(f"\nskipped (toolchain absent): {', '.join(skipped)}")
    if failures:
        log("\n" + "\n".join(f"FAIL {failure}" for failure in failures))
        return 1
    log("\nall checks passed")
    return 0


def bench(impls: list[Impl], repeat: int, skip_missing: bool, pin_cpu: bool,
          out_path: str | None) -> int:
    expected = load_expected()
    goldens = expected["impls"]
    results: dict[str, dict] = {}
    failures: list[str] = []

    for impl in impls:
        absent = missing_tools(impl)
        if absent:
            message = f"{impl.name}: missing {', '.join(absent)}"
            if not skip_missing:
                failures.append(message)
                continue
            log(f"SKIP {message}")
            results[impl.name] = {
                "label": impl.label.format(v="n/a"),
                "status": "missing",
                "in_bench": impl.in_bench,
                "threaded": impl.threaded,
            }
            continue

        version = detect_version(impl)
        try:
            build(impl)
            timings = []
            counts = None
            for _ in range(repeat):
                counts, elapsed = execute(impl, pin_cpu)
                timings.append(elapsed)
        except Failure as err:
            failures.append(str(err))
            continue

        golden = goldens.get(impl.name)
        if golden is not None and list(counts) != list(golden):
            failures.append(
                f"{impl.name}: got {'/'.join(map(str, counts))}, "
                f"expected {'/'.join(map(str, golden))}"
            )
            continue

        results[impl.name] = {
            "label": impl.label.format(v=version),
            "version": version,
            "status": "ok",
            "best_ms": min(timings),
            "worst_ms": max(timings),
            "runs_ms": timings,
            "counts": list(counts),
            "in_bench": impl.in_bench,
            "threaded": impl.threaded,
        }
        log(f"ok   {impl.name:8s} best {min(timings):g} ms  "
            f"(worst {max(timings):g}, n={repeat})")

    if failures:
        log("\n" + "\n".join(f"FAIL {failure}" for failure in failures))
        return 1

    payload = {
        "arch": platform.machine(),
        "runner": os.environ.get("RUNNER_LABEL", platform.node()),
        "cpu": cpu_model(),
        "cores": os.cpu_count(),
        "kernel": platform.release(),
        "commit": git_commit(),
        "date": datetime.now(timezone.utc).strftime("%Y-%m-%d"),
        "repeat": repeat,
        "pinned": pin_cpu,
        "results": results,
    }

    text = json.dumps(payload, indent=2, sort_keys=True) + "\n"
    if out_path:
        with open(out_path, "w") as handle:
            handle.write(text)
        log(f"\nwrote {out_path}")
    else:
        print(text)
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true",
                      help="verify output against ci/expected.json")
    mode.add_argument("--bench", action="store_true",
                      help="time each implementation and emit JSON")
    parser.add_argument("--only", help="comma-separated implementation names")
    parser.add_argument("--repeat", type=int, default=5,
                        help="benchmark repetitions (default: 5)")
    parser.add_argument("--skip-missing", action="store_true",
                        help="skip implementations whose toolchain is absent")
    parser.add_argument("--pin-cpu", action="store_true",
                        help="pin single-threaded runs to one core via taskset")
    parser.add_argument("--update-goldens", action="store_true",
                        help="rewrite ci/expected.json from this run (--check)")
    parser.add_argument("--out", help="write benchmark JSON here (--bench)")
    args = parser.parse_args()

    if args.update_goldens and not args.check:
        parser.error("--update-goldens requires --check")
    if args.repeat < 1:
        parser.error("--repeat must be at least 1")

    impls = selected(args.only)
    if args.check:
        return check(impls, args.skip_missing, args.update_goldens)
    return bench(impls, args.repeat, args.skip_missing, args.pin_cpu, args.out)


if __name__ == "__main__":
    sys.exit(main())
