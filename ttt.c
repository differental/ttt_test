// Credit: Jeremy Chen (gh:intgrah)
/* ttt_test - Speed test for random games of tic tac toe
 * Released into the public domain by Brian Chen (differental) and Jeremy Chen (intgrah), 2025.
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or distribute
 * this software, either in source code form or as a compiled binary, for any
 * purpose, commercial or non-commercial, and by any means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors of
 * this software dedicate any and all copyright interest in the software to
 * the public domain. We make this dedication for the benefit of the public at
 * large and to the detriment of our heirs and successors. We intend this
 * dedication to be an overt act of relinquishment in perpetuity of all
 * present and future rights to this software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <https://unlicense.org/>
 */

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define BOARD_SIZE 20
#define BOARD_PADDED (BOARD_SIZE + 2)
#define BOARD_SIZE_SQUARED (BOARD_SIZE * BOARD_SIZE)
#define WIN_CONDITION 10
typedef unsigned char u8;

// ================================================================
// Compass directions: East is +x, South is +y
//
//   N
// W   E
//   S
//
// Board representation (row major)
// Point (x, y) lies at arr[y + 1][x + 1]
// Have padding of 1 cell on all sides to avoid bounds checking.
//
// \ 0 1 2 3 4 x
// 0 . . . . .
// 1 . . . . .
// 2 . . . . .
// 3 . . . . .
// 4 . . . . .
// y
//
// Strategy:
// Each cell describes the number of filled squares in all eight directions.
//
// E.g.
//
// <- W . . # # . # # # . . -> E
//        p     q       r
//
// Cell p has p.e = 2
// Cell q has q.w = 2, q.e = 3
// Cell r has r.w = 3
//
// If q becomes filled:
//   The row is joined and now has length k = q.w + 1 + q.e
//   If k >= WIN_CONDITION,
//     then q forms a winning line.
//   Otherwise,
//     Set p.e to k
//     Set r.w to k
//   There is no need to update q.w or q.e in the future, even if p or r are filled, since
//   q.w and q.e will never be checked again.
// ================================================================

typedef struct
{
    u8 s, n, e, w, se, nw, ne, sw;
} Cell;

typedef Cell Board[BOARD_PADDED][BOARD_PADDED];

bool check_win(Board b, u8 x, u8 y)
{
    Cell q = b[y][x];

    u8 col = q.s + 1 + q.n;
    u8 row = q.w + 1 + q.e;
    u8 diag = q.nw + 1 + q.se;
    u8 anti = q.ne + 1 + q.sw;

    if (col >= WIN_CONDITION || row >= WIN_CONDITION || diag >= WIN_CONDITION || anti >= WIN_CONDITION)
        return true;

    b[y + q.s + 1][x].n = b[y - q.n - 1][x].s = col;
    b[y][x + q.e + 1].w = b[y][x - q.w - 1].e = row;
    b[y + q.se + 1][x + q.se + 1].nw = b[y - q.nw - 1][x - q.nw - 1].se = diag;
    b[y - q.ne - 1][x + q.ne + 1].sw = b[y + q.sw + 1][x - q.sw - 1].ne = anti;

    return false;
}

unsigned int xorshift(unsigned int max)
{
    // Xorshift
    static unsigned long x = 1729163UL;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    x &= 0xffffffffU;
    return x * max >> 32;
}

int do_game()
{
    u8 free_x[BOARD_SIZE_SQUARED];
    u8 free_y[BOARD_SIZE_SQUARED];
    for (int y = 1, i = 0; y <= BOARD_SIZE; y++)
        for (int x = 1; x <= BOARD_SIZE; x++, i++)
        {
            int j = xorshift(i + 1);
            free_x[i] = free_x[j];
            free_y[i] = free_y[j];
            free_x[j] = x;
            free_y[j] = y;
        }

    Board circle = {}, cross = {};

    for (int i = 0; i < BOARD_SIZE_SQUARED; i++)
    {
        int x = free_x[i];
        int y = free_y[i];

        if (i % 2 == 0)
        {
            if (check_win(circle, x, y))
                return 0;
        }
        else
        {
            if (check_win(cross, x, y))
                return 1;
        }
    }
    return -1;
}

int main()
{
    int n = 10000;
    int o = 0, x = 0, draw = 0;

    clock_t start = clock();

    for (int i = 0; i < n; i++)
    {
        int res = do_game();
        if (res == 0)
            o++;
        else if (res == 1)
            x++;
        else
            draw++;
    }

    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("O/X/Draw: %d/%d/%d\n", o, x, draw);
    printf("Time taken: %d ms\n", (int)(time_taken * 1000));
    return 0;
}
