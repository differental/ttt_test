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

const BOARD_SIZE: usize = 20;
const BOARD_PADDED: usize = BOARD_SIZE + 2;
const BOARD_SIZE_SQUARED: usize = BOARD_SIZE * BOARD_SIZE;
const WIN_CONDITION: usize = 10;

struct Cell {
    n: usize,
    s: usize,
    e: usize,
    w: usize,
    ne: usize,
    se: usize,
    sw: usize,
    nw: usize,
}

struct Board {
    cells: [[Cell; BOARD_PADDED]; BOARD_PADDED],
}

impl Board {
    fn new() -> Self {
        Self {
            cells: std::array::from_fn(|_| {
                std::array::from_fn(|_| Cell {
                    n: 0,
                    s: 0,
                    e: 0,
                    w: 0,
                    ne: 0,
                    se: 0,
                    sw: 0,
                    nw: 0,
                })
            }),
        }
    }

    fn check_win(&mut self, x: usize, y: usize) -> bool {
        let q = &raw const self.cells[y][x];
        let q = unsafe { &*q };

        let col = q.s + 1 + q.n;
        let row = q.w + 1 + q.e;
        let diag = q.nw + 1 + q.se;
        let anti = q.ne + 1 + q.sw;

        if col >= WIN_CONDITION
            || row >= WIN_CONDITION
            || diag >= WIN_CONDITION
            || anti >= WIN_CONDITION
        {
            return true;
        }

        self.cells[y + q.s + 1][x].n = col;
        self.cells[y - q.n - 1][x].s = col;

        self.cells[y][x + q.e + 1].w = row;
        self.cells[y][x - q.w - 1].e = row;

        self.cells[y + q.se + 1][x + q.se + 1].nw = diag;
        self.cells[y - q.nw - 1][x - q.nw - 1].se = diag;

        self.cells[y - q.ne - 1][x + q.ne + 1].sw = anti;
        self.cells[y + q.sw + 1][x - q.sw - 1].ne = anti;

        false
    }
}

struct XorShift {
    x: u64,
}

impl XorShift {
    fn new(seed: u64) -> Self {
        Self { x: seed }
    }

    fn rand(&mut self, max: u32) -> u64 {
        self.x ^= self.x << 13;
        self.x ^= self.x >> 17;
        self.x ^= self.x << 5;
        self.x &= 0xffffffff;
        self.x * max as u64 >> 32
    }
}

enum Player {
    Circle,
    Cross,
}

fn do_game(rng: &mut XorShift) -> Option<Player> {
    let mut free_cells = [(0, 0); BOARD_SIZE_SQUARED];
    let mut i: usize = 0;
    for y in 1..=BOARD_SIZE {
        for x in 1..=BOARD_SIZE {
            let j = rng.rand((i + 1) as u32) as usize;
            free_cells[i] = free_cells[j];
            free_cells[j] = (x, y);
            i += 1;
        }
    }

    let mut circle = Board::new();
    let mut cross = Board::new();

    for (i, &(x, y)) in free_cells.iter().enumerate() {
        let player = if i % 2 == 0 {
            Player::Circle
        } else {
            Player::Cross
        };

        let board = match player {
            Player::Circle => &mut circle,
            Player::Cross => &mut cross,
        };

        if board.check_win(x, y) {
            return Some(player);
        }
    }

    None
}

fn main() {
    let mut rng = XorShift::new(1729);

    let n = 10000;
    let mut o = 0;
    let mut x = 0;
    let mut draw = 0;

    let start = std::time::Instant::now();

    for _ in 0..n {
        match do_game(&mut rng) {
            Some(Player::Circle) => o += 1,
            Some(Player::Cross) => x += 1,
            None => draw += 1,
        }
    }

    let duration = start.elapsed();
    println!("O/X/Draw: {}/{}/{}", o, x, draw);
    println!("Time taken: {} ms", duration.as_millis());
}
