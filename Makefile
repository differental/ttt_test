CFLAGS := -O3 -Wall -Wextra

all: rust cpp c go ocaml java lean

bench:
	@echo "Rust"
	@./ttt.rs.exe
	@echo "C++"
	@./ttt.cc.exe
	@echo "C"
	@./ttt.c.exe
	@echo "Go"
	@./ttt.go.exe
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
	g++ -o ttt.cc.exe ttt.cc $(CFLAGS)

go:
	go build -o ttt.go.exe -ldflags="-s -w" ttt.go

c:
	gcc -o ttt.c.exe ttt.c $(CFLAGS)

ocaml:
	ocamlopt -o ttt.ml.exe ttt.ml -O3

java:
	javac ttt.java -Xlint -g:none

lean: LEAN := $(shell lean --print-prefix)
lean:
	lean ttt.lean -c ttt.lean.c
	$(LEAN)/bin/clang -o ttt.lean.exe ttt.lean.c $(CFLAGS) --sysroot=$(LEAN) \
	-I $(LEAN)/include -fPIC -fvisibility=hidden -isystem $(LEAN)/include/clang -DNDEBUG \
	-L $(LEAN)/lib/glibc -lc_nonshared -lpthread_nonshared \
	-L $(LEAN)/lib -Wl,-Bstatic -lgmp -lunwind -luv \
	-fuse-ld=lld -L $(LEAN)/lib/lean -lleancpp -lLean -lStd -lInit -lleanrt -lc++ -lc++abi -Wl,-Bdynamic -lm
