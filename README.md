# Bench

# Tic-Tac-Toe Benchmark

This repository contains implementations of a Tic-Tac-Toe game simulation in various programming languages to compare their performance.

## Benchmark Results

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

## How to Run

Use the Makefile to compile and run each implementation:
