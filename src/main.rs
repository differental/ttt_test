use rand::seq::SliceRandom;

const BOARD_SIZE: usize = 20;
const BOARD_SIZE_SQUARED: usize = BOARD_SIZE * BOARD_SIZE;
const WIN_CONDITION: usize = 10;

type Board = [[bool; BOARD_SIZE as usize]; BOARD_SIZE as usize];

fn check_win(
    x: isize,
    y: isize,
    b: &Board,
    cols: [usize; BOARD_SIZE],
    rows: [usize; BOARD_SIZE],
) -> bool {
    let mut acc;
    let mut xa: isize;
    let mut ya: isize;

    // Column
    if cols[x as usize] >= WIN_CONDITION {
        acc = 1;
        ya = y + 1;
        while ya < BOARD_SIZE as isize && b[ya as usize][x as usize] {
            acc += 1;
            if acc >= WIN_CONDITION {
                return true;
            }
            ya += 1;
        }
        ya = y - 1;
        while 0 <= ya && b[ya as usize][x as usize] {
            acc += 1;
            if acc >= WIN_CONDITION {
                return true;
            }
            ya -= 1;
        }
    }

    // Row
    if rows[y as usize] >= WIN_CONDITION {
        acc = 1;
        xa = x + 1;
        while xa < BOARD_SIZE as isize && b[y as usize][xa as usize] {
            acc += 1;
            if acc >= WIN_CONDITION {
                return true;
            }
            xa += 1;
        }
        xa = x - 1;
        while 0 <= xa && b[y as usize][xa as usize] {
            acc += 1;
            if acc >= WIN_CONDITION {
                return true;
            }
            xa -= 1;
        }
    }

    // Diagonal
    if BOARD_SIZE as isize - (x - y).abs() >= WIN_CONDITION as isize {
        acc = 1;
        xa = x + 1;
        ya = y + 1;
        while xa < BOARD_SIZE as isize && ya < BOARD_SIZE as isize && b[ya as usize][xa as usize] {
            acc += 1;
            if acc >= WIN_CONDITION {
                return true;
            }
            xa += 1;
            ya += 1;
        }
        xa = x - 1;
        ya = y - 1;
        while 0 <= xa && 0 <= ya && b[ya as usize][xa as usize] {
            acc += 1;
            if acc >= WIN_CONDITION {
                return true;
            }
            xa -= 1;
            ya -= 1;
        }
    }

    // Anti-diagonal
    if BOARD_SIZE as isize - (x + y - BOARD_SIZE as isize + 1).abs() >= WIN_CONDITION as isize {
        acc = 1;
        xa = x + 1;
        ya = y - 1;
        while xa < BOARD_SIZE as isize && 0 <= ya && b[ya as usize][xa as usize] {
            acc += 1;
            if acc >= WIN_CONDITION {
                return true;
            }
            xa += 1;
            ya -= 1;
        }
        xa = x - 1;
        ya = y + 1;
        while 0 <= xa && ya < BOARD_SIZE as isize && b[ya as usize][xa as usize] {
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

fn do_game(rng: &mut rand::rngs::ThreadRng) -> Option<bool> {
    let mut ocols = [0; BOARD_SIZE];
    let mut orows = [0; BOARD_SIZE];
    let mut xcols = [0; BOARD_SIZE];
    let mut xrows = [0; BOARD_SIZE];
    let mut os = [[false; BOARD_SIZE]; BOARD_SIZE];
    let mut xs = [[false; BOARD_SIZE]; BOARD_SIZE];
    let mut free_cells: Vec<usize> = (0..BOARD_SIZE_SQUARED).collect();

    free_cells.shuffle(rng);

    for (i, &index) in free_cells.iter().enumerate() {
        let x = index % BOARD_SIZE;
        let y = index / BOARD_SIZE;
        if i % 2 == 0 {
            os[y][x] = true;
            ocols[x] += 1;
            orows[y] += 1;
            if check_win(x as isize, y as isize, &os, ocols, orows) {
                return Some(true);
            }
        } else {
            xs[y][x] = true;
            xcols[x] += 1;
            xrows[y] += 1;
            if check_win(x as isize, y as isize, &xs, xcols, xrows) {
                return Some(false);
            }
        }
    }

    None
}

fn main() {
    let mut rng = rand::rng();

    let n = 10000;
    let mut o = 0;
    let mut x = 0;
    let mut draw = 0;

    let start = std::time::Instant::now();

    for _ in 0..n {
        match do_game(&mut rng) {
            Some(true) => o += 1,
            Some(false) => x += 1,
            None => draw += 1,
        }
    }

    let duration = start.elapsed();
    println!("O/X/Draw: {}/{}/{}", o, x, draw);
    println!("Time taken: {:?}", duration);
}
