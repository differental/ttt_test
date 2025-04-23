const BOARD_SIZE = 20;
const BOARD_SIZE_SQUARED = BOARD_SIZE * BOARD_SIZE;
const WIN_CONDITION = 10;

class Board {
    constructor() {
        this.board = Array.from({ length: BOARD_SIZE }, () =>
            Array(BOARD_SIZE).fill(false)
        );
        this.cols = Array(BOARD_SIZE).fill(0);
        this.rows = Array(BOARD_SIZE).fill(0);
        this.diag = Array(2 * BOARD_SIZE - 1).fill(0);
        this.anti = Array(2 * BOARD_SIZE - 1).fill(0);
    }

    update(x, y) {
        this.board[y][x] = true;
        this.cols[x]++;
        this.rows[y]++;
        this.diag[x - y + BOARD_SIZE - 1]++;
        this.anti[x + y]++;
    }

    check_win(x, y) {
        // Column
        if (this.cols[x] >= WIN_CONDITION) {
            let acc = 1;
            for (let ya = y; ++ya < BOARD_SIZE && this.board[ya][x]; )
                if (++acc >= WIN_CONDITION) return true;
            for (let ya = y; 0 <= --ya && this.board[ya][x]; )
                if (++acc >= WIN_CONDITION) return true;
        }

        // Row
        if (this.rows[y] >= WIN_CONDITION) {
            let acc = 1;
            for (let xa = x; ++xa < BOARD_SIZE && this.board[y][xa]; )
                if (++acc >= WIN_CONDITION) return true;
            for (let xa = x; 0 <= --xa && this.board[y][xa]; )
                if (++acc >= WIN_CONDITION) return true;
        }

        // Diagonal
        if (this.diag[x - y + BOARD_SIZE - 1] >= WIN_CONDITION) {
            let acc = 1;
            for (
                let xa = x, ya = y;
                ++xa < BOARD_SIZE && ++ya < BOARD_SIZE && this.board[ya][xa];

            )
                if (++acc >= WIN_CONDITION) return true;
            for (
                let xa = x, ya = y;
                0 <= --xa && 0 <= --ya && this.board[ya][xa];

            )
                if (++acc >= WIN_CONDITION) return true;
        }

        // Anti-diagonal
        if (this.anti[x + y] >= WIN_CONDITION) {
            let acc = 1;
            for (
                let xa = x, ya = y;
                ++xa < BOARD_SIZE && 0 <= --ya && this.board[ya][xa];

            )
                if (++acc >= WIN_CONDITION) return true;
            for (
                let xa = x, ya = y;
                0 <= --xa && ++ya < BOARD_SIZE && this.board[ya][xa];

            )
                if (++acc >= WIN_CONDITION) return true;
        }
        return false;
    }
}

function shuffle(array) {
    for (let i = array.length - 1; i > 0; i--) {
        let j = Math.floor(Math.random() * (i + 1));
        [array[i], array[j]] = [array[j], array[i]];
    }
}

function do_game() {
    const free_cells = Array(BOARD_SIZE_SQUARED)
        .fill(0)
        .map((_, i) => i);
    shuffle(free_cells);

    const circle = new Board();
    const cross = new Board();

    for (let i = 0; i < BOARD_SIZE_SQUARED; i++) {
        const x = free_cells[i] % BOARD_SIZE;
        const y = Math.floor(free_cells[i] / BOARD_SIZE);

        if (i % 2 === 0) {
            circle.update(x, y);
            if (circle.check_win(x, y)) {
                return 0;
            }
        } else {
            cross.update(x, y);
            if (cross.check_win(x, y)) {
                return 1;
            }
        }
    }

    return -1;
}

function main() {
    const n = 10000;
    let o = 0;
    let x = 0;
    let draw = 0;

    const start = performance.now();

    for (let i = 0; i < n; i++) {
        const res = do_game();
        if (res === 0) {
            o += 1;
        } else if (res === 1) {
            x += 1;
        } else {
            draw += 1;
        }
    }

    const end = performance.now();

    console.log(`O/X/Draw: ${o}/${x}/${draw}`);
    console.log(`Time taken: ${end - start} ms`);
}

main();
