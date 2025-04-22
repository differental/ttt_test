use rand::rngs::mock::StepRng;
use rand::seq::SliceRandom;
use rand::{Rng, SeedableRng};
use rand_xoshiro::{SplitMix64, Xoshiro256Plus};

const BOARD_SIZE: usize = 20;
const BOARD_SIZE_SQUARED: usize = BOARD_SIZE * BOARD_SIZE;
const WIN_CONDITION: usize = 10;

#[derive(Clone, Copy, PartialEq)]
enum Cell {
    None,
    Circle,
    Cross,
}

type Board = [[Cell; BOARD_SIZE]; BOARD_SIZE];

fn check_line(p: Cell, mut n: usize, dx: isize, dy: isize, mut x: isize, mut y: isize, board: &Board) -> bool {
    let mut acc = 0;
    while n > 0 {
        if let Some(row) = board.get(y as usize) {
            if let Some(&cell) = row.get(x as usize) {
                if cell == p {
                    acc += 1;
                    if acc == WIN_CONDITION {
                        return true;
                    }
                } else {
                    acc = 0;
                    if n - 1 < WIN_CONDITION {
                        return false;
                    }
                }
            } else {
                return false;
            }
        } else {
            return false;
        }
        x += dx;
        y += dy;
        n -= 1;
    }
    false
}

fn check_win(p: Cell, x: usize, y: usize, board: &Board) -> bool {
    let x = x as isize;
    let y = y as isize;

    // Column
    if check_line(p, BOARD_SIZE, 0, 1, x, 0, board) {
        return true;
    }
    // Row
    if check_line(p, BOARD_SIZE, 1, 0, 0, y, board) {
        return true;
    }
    // Diagonal
    let diag_len = BOARD_SIZE - (x - y).abs() as usize;
    let d = x.min(y);
    if diag_len >= WIN_CONDITION && check_line(p, diag_len, 1, 1, x - d, y - d, board) {
        return true;
    }
    // Anti-diagonal
    let diag_len = BOARD_SIZE - ((x + y) - (BOARD_SIZE as isize - 1)).abs() as usize;
    let d = ((BOARD_SIZE - 1) as isize - x).min(y);
    if diag_len >= WIN_CONDITION && check_line(p, diag_len, -1, 1, x + d, y - d, board) {
        return true;
    }

    false
}

fn do_game(rng: &mut StepRng) -> Cell {
    let mut board = [[Cell::None; BOARD_SIZE]; BOARD_SIZE];
    let mut free_cells: Vec<usize> = (0..BOARD_SIZE_SQUARED).collect();
    free_cells.shuffle(rng);

    for (i, &index) in free_cells.iter().enumerate() {
        let x = index / BOARD_SIZE;
        let y = index % BOARD_SIZE;
        let player = if i % 2 == 0 { Cell::Circle } else { Cell::Cross };
        board[y][x] = player;

        if check_win(player, x, y, &board) {
            return player;
        }
    }

    Cell::None
}

fn main() {
    let mut rng = StepRng::new(0, 1);
    let n = 100000;
    let mut o = 0;
    let mut x = 0;
    let mut draw = 0;

    for _ in 0..n {
        match do_game(&mut rng) {
            Cell::Circle => o += 1,
            Cell::Cross => x += 1,
            Cell::None => draw += 1,
        }
    }

    println!("O/X/Draw: {}/{}/{}", o, x, draw);
}
