all: rust cpp c

rust:
	cargo build --profile release-prod

cpp:
	g++ -Wall -Wextra -O3 ttt.cc -o ttt

go:
	go build -ldflags="-s -w" -o ttt ttt.go

c:
	gcc -Wall -Wextra -O3 ttt.c -o ttt

ocaml:
	ocamlopt -O3 ttt.ml -o ttt
	rm -f ttt.cmi
	rm -f ttt.cmx
	rm -f ttt.o

java:
	javac -Xlint -g:none ttt.java
	java ttt
	rm -f *.class

python:
	python3 ttt.py

pypy:
	pypy3 ttt.py

clean:
	rm -f ttt
