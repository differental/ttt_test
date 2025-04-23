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

python:
	python3 ttt.py

pypy:
	pypy3 ttt.py

cython:
	cython -Werror -Wextra --embed ttt.cpy -o ttt.cpy.c 
	gcc -Wall -Wextra -O3 -fPIC -I /usr/include/python3.13 -l python3.13 ttt.cpy.c -o ttt
	rm -f ttt.cpy.c

clean:
	rm -f ttt
