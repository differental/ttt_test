CFLAGS := -O3 -Wall -Wextra

rust:
	rustc ttt.rs -o ttt -C opt-level=3 -C target-cpu=native -C lto

cpp:
	g++ -o ttt ttt.cc $(CFLAGS)

go:
	go build -o ttt -ldflags="-s -w" ttt.go

c:
	gcc ttt.c -o ttt $(CFLAGS)

ocaml:
	ocamlopt -o ttt ttt.ml -O3
	rm -f ttt.cmi
	rm -f ttt.cmx
	rm -f ttt.o

java:
	javac ttt.java -Xlint -g:none
	java ttt
	rm -f *.class

js:
	node ttt.js

lean: LEAN := $(shell lean --print-prefix)
lean:
	lean ttt.lean -c ttt.lean.c
	$(LEAN)/bin/clang -o ttt ttt.lean.c $(CFLAGS) --sysroot=$(LEAN) \
	-I $(LEAN)/include -fPIC -fvisibility=hidden -isystem $(LEAN)/include/clang -DNDEBUG \
	-L $(LEAN)/lib/glibc -lc_nonshared -lpthread_nonshared \
	-L $(LEAN)/lib -Wl,-Bstatic -lgmp -lunwind -luv \
	-fuse-ld=lld -L $(LEAN)/lib/lean -lleancpp -lLean -lStd -lInit -lleanrt -lc++ -lc++abi -Wl,-Bdynamic -lm
	rm -f ttt.lean.c

python:
	python3 ttt.py

pypy:
	pypy3 ttt.py

clean:
	rm -f ttt
