CC := gcc
CFLAGS := -O3 -Wall -Wextra
CXXFLAGS := -O3 -march=native -funroll-loops -Wall -Wextra

.PHONY: all all-variants bench check clean rust cpp cpp-mt go c alt ocaml java lean

all: rust cpp c go ocaml java lean

# Everything in `all`, plus the two entries that are not part of `bench`:
# the multi-threaded C++ variant and the 40x40 alternative board.
all-variants: all cpp-mt alt

# Build and run every implementation, then check it produced the expected
# result. This is what CI runs; see ci/expected.json.
check:
	python3 ci/bench.py --check

bench:
	@echo "Rust"
	@./ttt.rs.exe
	@echo "C++"
	@./ttt.cc.exe
	@echo "C"
	@./ttt.c.exe
	@echo "Go"
	@CPUS=1 ./ttt.go.exe
	@echo "OCaml"
	@./ttt.ml.exe
	@echo "JavaScript"
	@node ttt.js
	@echo "Java"
	@java ttt
	@echo "Pypy"
	@pypy3 ttt.py
	@echo "Lean"
	@./ttt.lean.exe
	@echo "Python"
	@python3 ttt.py

clean:
	rm -f *.exe
	rm -f *.o
	rm -f *.class
	rm -f *.cmi
	rm -f *.cmx
	rm -f *.lean.c
	rm -f *.olean
	rm -f *.ilean

rust:
	rustc -o ttt.rs.exe ttt.rs -C opt-level=3 -C target-cpu=native -C lto

cpp:
	g++ -o ttt.cc.exe ttt.cc $(CXXFLAGS)

# Multi-threaded C++ variant. Same output, but not part of `bench`: every
# other entry here runs single-threaded, so it is not a fair comparison.
cpp-mt:
	g++ -o ttt-mt.cc.exe ttt-mt.cc $(CXXFLAGS) -pthread

go:
	go build -o ttt.go.exe -ldflags="-s -w" ttt.go

c:
	gcc -o ttt.c.exe ttt.c $(CFLAGS)

# Alternative implementation on a 40x40 board with a win condition of 12.
# A different game from every other entry here, so it is not benchmarked
# against them, but it is still built and checked.
alt:
	gcc -o ttt-alt.c.exe ttt-alt.c $(CFLAGS)

ocaml:
	ocamlopt -o ttt.ml.exe ttt.ml -O3

java:
	javac ttt.java -Xlint -g:none

lean: LEAN := $(shell lean --print-prefix)
lean:
	lean ttt.lean -c ttt.lean.c
	$(LEAN)/bin/clang -o ttt.lean.exe ttt.lean.c -O3 --sysroot=$(LEAN) \
	-I $(LEAN)/include -fPIC -fvisibility=hidden -isystem $(LEAN)/include/clang -DNDEBUG \
	-L $(LEAN)/lib/glibc -lc_nonshared -lpthread_nonshared \
	-L $(LEAN)/lib -Wl,-Bstatic -lgmp -lunwind -luv \
	-fuse-ld=lld -L $(LEAN)/lib/lean -lleancpp -lLean -lStd -lInit -lleanrt -lc++ -lc++abi -Wl,-Bdynamic -lm
