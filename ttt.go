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
	boardSizeSquared = boardSize * boardSize
	numDiagonals     = 2*boardSize - 1
	winCondition     = 10
)

type Board struct {
	board [boardSize][boardSize]bool
	cols  [boardSize]int
	rows  [boardSize]int
	diag  [numDiagonals]int
	anti  [numDiagonals]int
}

func NewBoard() *Board {
	return &Board{}
}

func (b *Board) update(x, y int) {
	b.board[y][x] = true
	b.cols[x]++
	b.rows[y]++
	b.diag[x-y+boardSize-1]++
	b.anti[x+y]++
}

func (b *Board) checkWin(x, y int) bool {
	var acc, xa, ya int
	// Column
	if b.cols[x] >= winCondition {
		acc = 1
		ya = y + 1
		for ya < boardSize && b.board[ya][x] {
			acc++
			if acc == winCondition {
				return true
			}
			ya++
		}
		ya = y - 1
		for 0 <= ya && b.board[ya][x] {
			acc++
			if acc == winCondition {
				return true
			}
			ya--
		}
	}

	// Row
	if b.rows[y] >= winCondition {
		acc = 1
		xa = x + 1
		for xa < boardSize && b.board[y][xa] {
			acc++
			if acc == winCondition {
				return true
			}
			xa++
		}
		xa = x - 1
		for 0 <= xa && b.board[y][xa] {
			acc++
			if acc == winCondition {
				return true
			}
			xa--
		}
	}

	// Diagonal
	if b.diag[x-y+boardSize-1] >= winCondition {
		acc = 1
		xa = x + 1
		ya = y + 1
		for xa < boardSize && ya < boardSize && b.board[ya][xa] {
			acc++
			if acc == winCondition {
				return true
			}
			xa++
			ya++
		}
		xa = x - 1
		ya = y - 1
		for 0 <= xa && 0 <= ya && b.board[ya][xa] {
			acc++
			if acc == winCondition {
				return true
			}
			xa--
			ya--
		}
	}

	// Anti-diagonal
	if b.anti[x+y] >= winCondition {
		acc = 1
		xa = x + 1
		ya = y - 1
		for xa < boardSize && ya >= 0 && b.board[ya][xa] {
			acc++
			if acc == winCondition {
				return true
			}
			xa++
			ya--
		}
		xa = x - 1
		ya = y + 1
		for xa >= 0 && ya < boardSize && b.board[ya][xa] {
			acc++
			if acc == winCondition {
				return true
			}
			xa--
			ya++
		}
	}

	return false
}

func doGame(r *rand.Rand) int {
	freeCells := make([]int, boardSizeSquared)
	for i := range freeCells {
		freeCells[i] = i
	}
	r.Shuffle(len(freeCells), func(i, j int) {
		freeCells[i], freeCells[j] = freeCells[j], freeCells[i]
	})

	circle := NewBoard()
	cross := NewBoard()

	for i := 0; i < boardSizeSquared; i++ {
		x := freeCells[i] % boardSize
		y := freeCells[i] / boardSize

		if i%2 == 0 {
			circle.update(x, y)
			if circle.checkWin(x, y) {
				return 0
			}
		} else {
			cross.update(x, y)
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
		r := rand.New(rand.NewSource(int64(1729 + i)))
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
