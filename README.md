# Bench

# Tic-Tac-Toe Benchmark

This repository contains implementations of a Tic-Tac-Toe game simulation in various programming languages to compare their performance.

Each implementation plays 10000 random games on a 20x20 board and reports the
win/draw split along with how long the games took. The seeds are fixed, so every
implementation is deterministic and CI can check that it still produces exactly
the right answer.

## Benchmark Results

### Reference machine

Hand-measured by the author. These are the numbers the implementations were
tuned against.

| Language                           | Execution Time (ms) |
| ---------------------------------- | ------------------- |
| Rust rustc 1.86.0 -O               | 46                  |
| C++ (g++) 14.2.1 -O3               | 48                  |
| C (gcc) 14.2.1 -O3                 | 53                  |
| Go 1.24.2 (single-threaded)        | 62                  |
| OCaml (ocamlopt flambda) 5.3.0 -O3 | 105                 |
| JavaScript (node.js) 23.9.0        | 130                 |
| Java (openjdk) 24.0.1              | 185                 |
| Python (PyPy) 7.3.19 + gcc 14.2.1  | 540                 |
| Lean 4.18.0 + Clang 19.1.7         | 1000                |
| Python (CPython) 3.13.3            | 1880                |

The Lean row is not currently reproducible: `ttt.lean` does not compile. See
[Continuous Integration](#continuous-integration) below.

### GitHub Actions, x86_64

Measured automatically by [`.github/workflows/bench.yml`](.github/workflows/bench.yml).

<!-- BENCH:x64:START -->
| Language | Best of 5 (ms) |
| -------- | ------------------- |
| C++ (g++ 13.3.0) -O3 | 21 |
| C (gcc 13.3.0) -O3 | 55 |
| Rust (rustc 1.98.1) -O3 | 55 |
| Go 1.24.2 (single-threaded) | 89 |
| OCaml (ocamlopt flambda 5.3.0) -O3 | 177 |
| JavaScript (node.js 23.9.0) | 190 |
| Java (openjdk 24.0.2) | 331 |
| Python (PyPy 8.0.0) | 807 |
| Python (CPython 3.13.15) | 3067 |

- Not comparable, verified but excluded: C++ multi-threaded (g++ 13.3.0) -O3 — 11 ms on 4 cores
- Not comparable, verified but excluded: C, 40x40 board (gcc 13.3.0) -O3 — 310 ms

Measured on `ubuntu-24.04` (AMD EPYC 7763 64-Core Processor, 4 cores), Linux 6.17.0-1022-azure, commit `4a1d94e`, 2026-10-05.

Best of 5 runs, pinned to a single core. Shared CI runners are noisy: treat differences under about 20% as
indistinguishable, and compare languages within a table rather than across
tables or against the reference machine above.
<!-- BENCH:x64:END -->

### GitHub Actions, aarch64

<!-- BENCH:arm64:START -->
| Language | Best of 5 (ms) |
| -------- | ------------------- |
| C++ (g++ 13.3.0) -O3 | 23 |
| Rust (rustc 1.98.1) -O3 | 35 |
| C (gcc 13.3.0) -O3 | 40 |
| Go 1.24.2 (single-threaded) | 78 |
| JavaScript (node.js 23.9.0) | 180 |
| Java (openjdk 24.0.2) | 287 |
| OCaml (ocamlopt flambda 5.3.0) -O3 | 354 |
| Python (PyPy 8.0.0) | 960 |
| Python (CPython 3.13.15) | 2640 |

- Not comparable, verified but excluded: C++ multi-threaded (g++ 13.3.0) -O3 — 6 ms on 4 cores
- Not comparable, verified but excluded: C, 40x40 board (gcc 13.3.0) -O3 — 271 ms

Measured on `ubuntu-24.04-arm` (aarch64, 4 cores), Linux 6.17.0-1022-azure, commit `4a1d94e`, 2026-10-05.

Best of 5 runs, pinned to a single core. Shared CI runners are noisy: treat differences under about 20% as
indistinguishable, and compare languages within a table rather than across
tables or against the reference machine above.
<!-- BENCH:arm64:END -->

## How to Run

Use the Makefile to compile and run each implementation:

```sh
make all      # build every benchmarked implementation
make bench    # run them all and print timings
make clean    # remove build artefacts
```

Individual targets are available too (`make rust`, `make cpp`, `make go`, ...).
Two extra implementations are built by `make all-variants` but left out of
`make bench`:

- `ttt-mt.cc` (`make cpp-mt`) is a multi-threaded C++ variant. Every other entry
  runs single-threaded, so including it would not be a fair comparison.
- `ttt-alt.c` (`make alt`) plays a different game: a 40x40 board with a win
  condition of 12.

## Continuous Integration

`make check` builds and runs every implementation and verifies its output
against the golden counts in [`ci/expected.json`](ci/expected.json). It asserts
that each one plays exactly 10000 games, matches its recorded result, and that
the five implementations sharing the same xorshift seed — C, C++, multi-threaded
C++, Go and OCaml — agree with each other exactly.

Run it locally with `python3 ci/bench.py --check --skip-missing`, which skips any
language whose toolchain you do not have installed.

### Known broken: Lean

`ttt.lean` does not compile on any Lean 4 toolchain, so it is excluded from both
the checks and the CI benchmark tables. `Board.update` is never defined,
`checkWin` is declared `(x y : Nat) (b : Board)` but called with five arguments,
and `shuffle` discharges its bounds obligations with `by sorry`. `checkWin`'s
`b.set` calls also discard their results, so the writes would be no-ops even if
it built.

It seeds the same xorshift with `1729163` as C, C++, Go and OCaml, so a working
version should produce `2567/2447/4986` and join `identical_group`. Once it does,
delete its `known_broken` entry in [`ci/expected.json`](ci/expected.json) and add
`lean` back to the CI matrix.
