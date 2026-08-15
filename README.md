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

### GitHub Actions, x86_64

Measured automatically by [`.github/workflows/bench.yml`](.github/workflows/bench.yml).

<!-- BENCH:x64:START -->
Not measured yet. Run the `bench` workflow to populate this table.
<!-- BENCH:x64:END -->

### GitHub Actions, aarch64

<!-- BENCH:arm64:START -->
Not measured yet. Run the `bench` workflow to populate this table.
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
the six implementations sharing the same xorshift seed — C, C++, multi-threaded
C++, Go, OCaml and Lean — agree with each other exactly.

Run it locally with `python3 ci/bench.py --check --skip-missing`, which skips any
language whose toolchain you do not have installed.
