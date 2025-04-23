all: rust cpp c

rust:
	cargo build --profile release-prod

cpp:
	g++ -O3 ttt.cc -o ttt

c:
	gcc -O3 ttt.c -o ttt
