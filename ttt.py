# ttt_test - Speed test for random games of tic tac toe
# Released into the public domain by Brian Chen (differental) and Jeremy Chen (intgrah), 2025.
#
# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or distribute
# this software, either in source code form or as a compiled binary, for any
# purpose, commercial or non-commercial, and by any means.
#
# In jurisdictions that recognize copyright laws, the author or authors of
# this software dedicate any and all copyright interest in the software to
# the public domain. We make this dedication for the benefit of the public at
# large and to the detriment of our heirs and successors. We intend this
# dedication to be an overt act of relinquishment in perpetuity of all
# present and future rights to this software under copyright law.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# For more information, please refer to <https://unlicense.org/>

import random
import time

BOARD_SIZE = 20
BOARD_SIZE_SQUARED = BOARD_SIZE * BOARD_SIZE
WIN_CONDITION = 10


class Board:
    board: list[list[bool]]
    cols: list[int]
    rows: list[int]
    diag: list[int]
    anti: list[int]

    def __init__(self):
        self.board = [[False] * BOARD_SIZE for _ in range(BOARD_SIZE)]
        self.cols = [0] * BOARD_SIZE
        self.rows = [0] * BOARD_SIZE
        self.diag = [0] * (2 * BOARD_SIZE - 1)
        self.anti = [0] * (2 * BOARD_SIZE - 1)

    def update(self, x, y):
        self.board[y][x] = True
        self.cols[x] += 1
        self.rows[y] += 1
        self.diag[x - y + BOARD_SIZE - 1] += 1
        self.anti[x + y] += 1

    def check_win(self, x, y) -> bool:
        # Column
        if self.cols[x] >= WIN_CONDITION:
            acc = 1
            ya = y + 1
            while ya < BOARD_SIZE and self.board[ya][x]:
                acc += 1
                ya += 1
            ya = y - 1
            while 0 <= ya and self.board[ya][x]:
                acc += 1
                ya -= 1
            if acc >= WIN_CONDITION:
                return True

        # Row
        if self.rows[y] >= WIN_CONDITION:
            acc = 1
            xa = x + 1
            while xa < BOARD_SIZE and self.board[y][xa]:
                acc += 1
                xa += 1
            xa = x - 1
            while 0 <= xa and self.board[y][xa]:
                acc += 1
                xa -= 1
            if acc >= WIN_CONDITION:
                return True

        # Diagonal
        if self.diag[x - y + BOARD_SIZE - 1] >= WIN_CONDITION:
            acc = 1
            xa = x + 1
            ya = y + 1
            while xa < BOARD_SIZE and ya < BOARD_SIZE and self.board[ya][xa]:
                acc += 1
                xa += 1
                ya += 1
            xa = x - 1
            ya = y - 1
            while 0 <= xa and 0 <= ya and self.board[ya][xa]:
                acc += 1
                xa -= 1
                ya -= 1
            if acc >= WIN_CONDITION:
                return True

        # Anti-diagonal
        if self.anti[x + y] >= WIN_CONDITION:
            acc = 1
            xa = x + 1
            ya = y - 1
            while xa < BOARD_SIZE and 0 <= ya and self.board[ya][xa]:
                acc += 1
                xa += 1
                ya -= 1
            xa = x - 1
            ya = y + 1
            while 0 <= xa and ya < BOARD_SIZE and self.board[ya][xa]:
                acc += 1
                xa -= 1
                ya += 1
            if acc >= WIN_CONDITION:
                return True

        return False


def do_game() -> int:
    free_cells: list[tuple[int, int]] = [
        (x, y) for y in range(BOARD_SIZE) for x in range(BOARD_SIZE)
    ]
    random.shuffle(free_cells)

    circle = Board()
    cross = Board()

    for i, (x, y) in enumerate(free_cells):
        if i % 2 == 0:
            circle.update(x, y)
            if circle.check_win(x, y):
                return 0
        else:
            cross.update(x, y)
            if cross.check_win(x, y):
                return 1

    return -1


def main() -> None:
    random.seed(1729)

    n = 10000
    o = 0
    x = 0
    draw = 0

    start: float = time.time()

    for _ in range(n):
        res: int = do_game()
        if res == 0:
            o += 1
        elif res == 1:
            x += 1
        else:
            draw += 1

    end: float = time.time()
    print(f"O/X/Draw: {o}/{x}/{draw}")
    print(f"Time taken: {(end - start) * 1000:.2f} ms")


if __name__ == "__main__":
    main()
