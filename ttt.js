const BOARD_SIZE = 20;
const BOARD_SIZE_SQUARED = BOARD_SIZE * BOARD_SIZE;
const WIN_CONDITION = 10;

class Board {
    constructor() {
        this.board = Array(BOARD_SIZE);
        for (let i = 0; i < BOARD_SIZE; i++) this.board[i] = Array(BOARD_SIZE);

        this.cols = Array(BOARD_SIZE);
        this.rows = Array(BOARD_SIZE);
        this.diag = Array(2 * BOARD_SIZE - 1);
        this.anti = Array(2 * BOARD_SIZE - 1);
    }

    clear() {
        for (let i = 0; i < BOARD_SIZE; i++) this.board[i].fill(false);
        this.cols.fill(0);
        this.rows.fill(0);
        this.diag.fill(0);
        this.anti.fill(0);
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

let x = 172916342314;
function rand() {
    // Xorshift
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    x &= 0xffffffff;
    return x;
}

function shuffle(array) {
    // Fisher-Yates
    for (let i = array.length - 1; i > 0; i--) {
        const m = i + 1;
        const j = Math.abs(rand()) % m;
        const temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

function do_game(free_cells, circle, cross) {
    for (let i = 0; i < BOARD_SIZE_SQUARED; i++) free_cells[i] = i;
    shuffle(free_cells);

    circle.clear();
    cross.clear();

    for (let i = 0; i < BOARD_SIZE_SQUARED; i++) {
        const x = free_cells[i] % BOARD_SIZE;
        const y = (free_cells[i] - x) / BOARD_SIZE;
        const player = i % 2;
        const board = player === 0 ? circle : cross;
        board.update(x, y);
        if (board.check_win(x, y)) return player;
    }

    return -1;
}

function main() {
    const n = 10000;
    let o = 0;
    let x = 0;
    let draw = 0;

    const free_cells = Array(BOARD_SIZE_SQUARED);
    const circle = new Board();
    const cross = new Board();

    const start = performance.now();

    for (let i = 0; i < n; i++) {
        const res = do_game(free_cells, circle, cross);
        switch (res) {
            case 0:
                o += 1;
                break;
            case 1:
                x += 1;
                break;
            case -1:
                draw += 1;
                break;
        }
    }

    const end = performance.now();

    console.log(`O/X/Draw: ${o}/${x}/${draw}`);
    console.log(`Time taken: ${Math.round(end - start)} ms`);
}

main();
