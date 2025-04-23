# cython: language_level=3, boundscheck=False
import random
import time
import cython
from array import array

# Constants
BOARD_SIZE = 20
BOARD_SIZE_SQUARED = BOARD_SIZE * BOARD_SIZE
WIN_CONDITION = 10


@cython.cclass
class Board:
    board: list[list[bool]]
    cols: array
    rows: array
    diag: array
    anti: array

    def __init__(self):
        # Initialize 2D board with Python lists (since array doesn't support 2D)
        self.board = [[False] * BOARD_SIZE for _ in range(BOARD_SIZE)]
        # Use array for 1D data
        self.cols = array("i", [0] * BOARD_SIZE)
        self.rows = array("i", [0] * BOARD_SIZE)
        self.diag = array("i", [0] * (2 * BOARD_SIZE - 1))
        self.anti = array("i", [0] * (2 * BOARD_SIZE - 1))

    @cython.locals(x=cython.int, y=cython.int)
    def update(self, x: int, y: int) -> None:
        self.board[y][x] = True
        self.cols[x] += 1
        self.rows[y] += 1
        self.diag[x - y + BOARD_SIZE - 1] += 1
        self.anti[x + y] += 1

    @cython.locals(
        x=cython.int, y=cython.int, acc=cython.int, xa=cython.int, ya=cython.int
    )
    def check_win(self, x: int, y: int) -> bool:
        # Column
        if self.cols[x] >= WIN_CONDITION:
            acc = 1
            ya = y + 1
            while ya < BOARD_SIZE and self.board[ya][x]:
                acc += 1
                if acc == WIN_CONDITION:
                    return True
                ya += 1
            ya = y - 1
            while 0 <= ya and self.board[ya][x]:
                acc += 1
                if acc == WIN_CONDITION:
                    return True
                ya -= 1

        # Row
        if self.rows[y] >= WIN_CONDITION:
            acc = 1
            xa = x + 1
            while xa < BOARD_SIZE and self.board[y][xa]:
                acc += 1
                if acc == WIN_CONDITION:
                    return True
                xa += 1
            xa = x - 1
            while 0 <= xa and self.board[y][xa]:
                acc += 1
                if acc == WIN_CONDITION:
                    return True
                xa -= 1

        # Diagonal
        if self.diag[x - y + BOARD_SIZE - 1] >= WIN_CONDITION:
            acc = 1
            xa = x + 1
            ya = y + 1
            while xa < BOARD_SIZE and ya < BOARD_SIZE and self.board[ya][xa]:
                acc += 1
                if acc == WIN_CONDITION:
                    return True
                xa += 1
                ya += 1
            xa = x - 1
            ya = y - 1
            while 0 <= xa and 0 <= ya and self.board[ya][xa]:
                acc += 1
                if acc == WIN_CONDITION:
                    return True
                xa -= 1
                ya -= 1

        # Anti-diagonal
        if self.anti[x + y] >= WIN_CONDITION:
            acc = 1
            xa = x + 1
            ya = y - 1
            while xa < BOARD_SIZE and 0 <= ya and self.board[ya][xa]:
                acc += 1
                if acc == WIN_CONDITION:
                    return True
                xa += 1
                ya -= 1
            xa = x - 1
            ya = y + 1
            while 0 <= xa and ya < BOARD_SIZE and self.board[ya][xa]:
                acc += 1
                if acc == WIN_CONDITION:
                    return True
                xa -= 1
                ya += 1

        return False


@cython.locals(i=cython.int, x=cython.int, y=cython.int)
def do_game() -> int:
    free_cells = list(range(BOARD_SIZE_SQUARED))
    random.shuffle(free_cells)

    circle = Board()
    cross = Board()

    for i in range(BOARD_SIZE_SQUARED):
        x = free_cells[i] % BOARD_SIZE
        y = free_cells[i] // BOARD_SIZE

        if i % 2 == 0:
            circle.update(x, y)
            if circle.check_win(x, y):
                return 0
        else:
            cross.update(x, y)
            if cross.check_win(x, y):
                return 1

    return -1


@cython.locals(n=cython.int, o=cython.int, x=cython.int, draw=cython.int)
def main() -> None:
    random.seed(1729)

    n = 10000
    o = 0
    x = 0
    draw = 0

    start = time.time()

    for _ in range(n):
        res = do_game()
        if res == 0:
            o += 1
        elif res == 1:
            x += 1
        else:
            draw += 1

    end = time.time()
    print(f"O/X/Draw: {o}/{x}/{draw}")
    print(f"Time taken: {int((end - start) * 1000)} ms")


if __name__ == "__main__":
    main()
