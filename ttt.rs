const BOARD_SIZE: usize = 20;
const BOARD_SIZE_SQUARED: usize = BOARD_SIZE * BOARD_SIZE;
const NUM_DIAGONALS: usize = 2 * BOARD_SIZE - 1;
const WIN_CONDITION: usize = 10;

struct Board {
    board: [[bool; BOARD_SIZE]; BOARD_SIZE],
    cols: [usize; BOARD_SIZE],
    rows: [usize; BOARD_SIZE],
    diag: [usize; NUM_DIAGONALS],
    anti: [usize; NUM_DIAGONALS],
}

impl Board {
    fn new() -> Self {
        Self {
            cols: [0; BOARD_SIZE],
            rows: [0; BOARD_SIZE],
            diag: [0; NUM_DIAGONALS],
            anti: [0; NUM_DIAGONALS],
            board: [[false; BOARD_SIZE]; BOARD_SIZE],
        }
    }

    fn update(&mut self, x: usize, y: usize) {
        self.board[y][x] = true;
        self.cols[x] += 1;
        self.rows[y] += 1;
        self.diag[BOARD_SIZE - 1 + x - y] += 1;
        self.anti[x + y] += 1;
    }

    fn check_win(&self, x: isize, y: isize) -> bool {
        let mut acc;
        let mut xa: isize;
        let mut ya: isize;

        // Column
        if self.cols[x as usize] >= WIN_CONDITION {
            acc = 1;
            ya = y + 1;
            while ya < BOARD_SIZE as isize && self.board[ya as usize][x as usize] {
                acc += 1;
                if acc >= WIN_CONDITION {
                    return true;
                }
                ya += 1;
            }
            ya = y - 1;
            while 0 <= ya && self.board[ya as usize][x as usize] {
                acc += 1;
                if acc >= WIN_CONDITION {
                    return true;
                }
                ya -= 1;
            }
        }

        // Row
        if self.rows[y as usize] >= WIN_CONDITION {
            acc = 1;
            xa = x + 1;
            while xa < BOARD_SIZE as isize && self.board[y as usize][xa as usize] {
                acc += 1;
                if acc >= WIN_CONDITION {
                    return true;
                }
                xa += 1;
            }
            xa = x - 1;
            while 0 <= xa && self.board[y as usize][xa as usize] {
                acc += 1;
                if acc >= WIN_CONDITION {
                    return true;
                }
                xa -= 1;
            }
        }

        // Diagonal
        if self.diag[BOARD_SIZE - 1 + x as usize - y as usize] >= WIN_CONDITION {
            acc = 1;
            xa = x + 1;
            ya = y + 1;
            while xa < BOARD_SIZE as isize
                && ya < BOARD_SIZE as isize
                && self.board[ya as usize][xa as usize]
            {
                acc += 1;
                if acc >= WIN_CONDITION {
                    return true;
                }
                xa += 1;
                ya += 1;
            }
            xa = x - 1;
            ya = y - 1;
            while 0 <= xa && 0 <= ya && self.board[ya as usize][xa as usize] {
                acc += 1;
                if acc >= WIN_CONDITION {
                    return true;
                }
                xa -= 1;
                ya -= 1;
            }
        }

        // Anti-diagonal
        if self.anti[x as usize + y as usize] >= WIN_CONDITION {
            acc = 1;
            xa = x + 1;
            ya = y - 1;
            while xa < BOARD_SIZE as isize && 0 <= ya && self.board[ya as usize][xa as usize] {
                acc += 1;
                if acc >= WIN_CONDITION {
                    return true;
                }
                xa += 1;
                ya -= 1;
            }
            xa = x - 1;
            ya = y + 1;
            while 0 <= xa && ya < BOARD_SIZE as isize && self.board[ya as usize][xa as usize] {
                acc += 1;
                if acc >= WIN_CONDITION {
                    return true;
                }
                xa -= 1;
                ya += 1;
            }
        }

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

    fn next(&mut self) -> u32 {
        self.x ^= self.x << 13;
        self.x ^= self.x >> 17;
        self.x ^= self.x << 5;
        self.x &= 0xffffffff;
        self.x as u32
    }

    fn next_range(&mut self, max: usize) -> usize {
        (self.next() as usize) % max
    }
}

fn shuffle<T>(rng: &mut XorShift, slice: &mut [T]) {
    // Fisher-Yates
    for i in (1..slice.len()).rev() {
        let j = rng.next_range(i + 1);
        slice.swap(i, j);
    }
}

enum Player {
    Circle,
    Cross,
}

fn do_game(rng: &mut XorShift) -> Option<Player> {
    let mut circle = Board::new();
    let mut cross = Board::new();
    let mut free_cells: Vec<usize> = (0..BOARD_SIZE_SQUARED).collect();

    shuffle(rng, &mut free_cells);

    for (i, &index) in free_cells.iter().enumerate() {
        let x = index % BOARD_SIZE;
        let y = index / BOARD_SIZE;

        let player = if i % 2 == 0 {
            Player::Circle
        } else {
            Player::Cross
        };

        let board = match player {
            Player::Circle => &mut circle,
            Player::Cross => &mut cross,
        };

        board.update(x, y);
        if board.check_win(x as isize, y as isize) {
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
