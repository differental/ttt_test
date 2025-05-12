// ttt_test - Speed test for random games of tic tac toe
// Released into the public domain by Brian Chen (differental) and Jeremy Chen (intgrah), 2025.
//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or distribute
// this software, either in source code form or as a compiled binary, for any
// purpose, commercial or non-commercial, and by any means.
//
// In jurisdictions that recognize copyright laws, the author or authors of
// this software dedicate any and all copyright interest in the software to
// the public domain. We make this dedication for the benefit of the public at
// large and to the detriment of our heirs and successors. We intend this
// dedication to be an overt act of relinquishment in perpetuity of all
// present and future rights to this software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <https://unlicense.org/>

package main

import (
	"fmt"
	"math/rand"
	"os"
	"runtime"
	"strconv"
	"sync"
	"time"
)

const (
	boardSize        = 20
	boardPadded      = boardSize + 2
	boardSizeSquared = boardSize * boardSize
	numDiagonals     = 2*boardSize - 1
	winCondition     = 10
)

type Cell struct {
	s, n, e, w, se, nw, ne, sw int
}

type Board [boardPadded][boardPadded]Cell

func NewBoard() *Board {
	return &Board{}
}

func (b *Board) checkWin(x, y int) bool {
	q := b[y][x]
	col := q.s + 1 + q.n
	row := q.w + 1 + q.e
	diag := q.nw + 1 + q.se
	anti := q.ne + 1 + q.sw

	if col >= winCondition || row >= winCondition || diag >= winCondition || anti >= winCondition {
		return true
	}

	b[y+q.s+1][x].n = col
	b[y-q.n-1][x].s = col
	b[y][x+q.e+1].w = row
	b[y][x-q.w-1].e = row
	b[y+q.se+1][x+q.se+1].nw = diag
	b[y-q.nw-1][x-q.nw-1].se = diag
	b[y-q.ne-1][x+q.ne+1].sw = anti
	b[y+q.sw+1][x-q.sw-1].ne = anti

	return false
}

var x = 1729163

func xorshift(max int) int32 {
	x ^= x << 13
	x ^= x >> 17
	x ^= x << 5
	x &= 0xffffffff
	return int32((x * max) >> 32)
}

func doGame(r *rand.Rand) int {
	freeCells := make([]int, boardSizeSquared)
	for i := range boardSizeSquared {
		j := xorshift(i + 1)
		freeCells[i] = freeCells[j]
		freeCells[j] = i
	}

	circle := NewBoard()
	cross := NewBoard()

	for i := 0; i < boardSizeSquared; i++ {
		x := freeCells[i]%boardSize + 1
		y := freeCells[i]/boardSize + 1

		if i%2 == 0 {
			if circle.checkWin(x, y) {
				return 0
			}
		} else {
			if cross.checkWin(x, y) {
				return 1
			}
		}
	}

	return -1
}

type Result struct {
	o, x, draw int
}

func worker(id int, games int, r *rand.Rand, wg *sync.WaitGroup, results chan<- Result) {
	defer wg.Done()

	o := 0
	x := 0
	draw := 0

	for i := 0; i < games; i++ {
		res := doGame(r)
		switch res {
		case 0:
			o++
		case 1:
			x++
		case -1:
			draw++
		}
	}

	results <- Result{o, x, draw}
}

func main() {
	// Default to all available CPUs
	cpus := runtime.NumCPU()
	if envCpus := os.Getenv("CPUS"); envCpus != "" {
		n, err := strconv.Atoi(envCpus)
		if err != nil || n <= 0 {
			fmt.Printf("1 <= CPUS <= %d\n", cpus)
			os.Exit(1)
		}
		cpus = n
	}

	n := 10000
	q, r := n/cpus, n%cpus

	var wg sync.WaitGroup
	results := make(chan Result, cpus)

	start := time.Now()

	for i := 0; i < cpus; i++ {
		wg.Add(1)
		games := q
		if i == 0 {
			games += r
		}
		r := rand.New(rand.NewSource(int64(1729163 + i)))
		go worker(i, games, r, &wg, results)
	}

	go func() {
		wg.Wait()
		close(results)
	}()

	var total Result

	for result := range results {
		total.o += result.o
		total.x += result.x
		total.draw += result.draw
	}

	duration := time.Since(start)
	fmt.Printf("O/X/Draw: %d/%d/%d\n", total.o, total.x, total.draw)
	fmt.Printf("Time taken: %d ms\n", duration.Milliseconds())
	fmt.Printf("Cores: %d\n", cpus)
}
