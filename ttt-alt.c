// Credit: Jeremy Chen (gh:intgrah)

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

#define BOARD_SIZE 40
#define BOARD_SIZE_SQUARED (BOARD_SIZE * BOARD_SIZE)
#define WIN_CONDITION 12

typedef struct
{
    int n[BOARD_SIZE][BOARD_SIZE];
    int s[BOARD_SIZE][BOARD_SIZE];
    int e[BOARD_SIZE][BOARD_SIZE];
    int w[BOARD_SIZE][BOARD_SIZE];
    int ne[BOARD_SIZE][BOARD_SIZE];
    int se[BOARD_SIZE][BOARD_SIZE];
    int sw[BOARD_SIZE][BOARD_SIZE];
    int nw[BOARD_SIZE][BOARD_SIZE];
} Board;

void board_init(Board *b)
{
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            b->n[i][j] = b->e[i][j] = b->s[i][j] = b->w[i][j] = b->ne[i][j] = b->se[i][j] = b->sw[i][j] = b->nw[i][j] = 1;
}

// ================================================================
// Compass directions: East is +x, South is +y
//
//   N
// W   E
//   S
//
// Board representation (row major)
// Point (x, y) lies at arr[y][x]
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
// Each b->_ describes the number of filled squares in that direction, + 1.
// E.g.
//
// . . # # . # # # . .
//   p     q       r
//
// Cell q has b->w[qy][qx] = 3, b->e[qy][qx] = 4
//
// If that cell were to become filled, simply check if w - 1 + e >= WIN_CONDITION.
//
// If not a winning line:
//   Update the b->e value for p
//   Update the b->w value for r
//
// p is at coordinate (qx - b->w[qy][qx], qy)
// r is at coordinate (qx + b->e[qy][qx], qy)
//
// Similarly for columns, diagonals, anti-diagonals
// ================================================================

inline bool valid(int i) { return 0 <= i && i < BOARD_SIZE; }

bool board_update(Board *b, int x, int y)
{
    if (!valid(x) || !valid(y))
    {
        printf("Precondition");
        exit(1);
    }
    // Check if S - 1 + N is a win
    if (b->s[y][x] - 1 + b->n[y][x] >= WIN_CONDITION)
        return true;
    // Check if W - 1 + E is a win
    if (b->w[y][x] - 1 + b->e[y][x] >= WIN_CONDITION)
        return true;
    // Check if NW - 1 + SE is a win
    if (b->nw[y][x] - 1 + b->se[y][x] >= WIN_CONDITION)
        return true;
    // Check if NE - 1 + SW is a win
    if (b->ne[y][x] - 1 + b->sw[y][x] >= WIN_CONDITION)
        return true;

    // ---- Column ----
    int ys = y + b->s[y][x];
    if (valid(ys))
        b->n[ys][x] += b->n[y][x];
    int yn = y - b->n[y][x];
    if (valid(yn))
        b->s[yn][x] += b->s[y][x];

    // ---- Row ----
    int xe = x + b->e[y][x];
    if (valid(xe))
        b->w[y][xe] += b->w[y][x];
    int xw = x - b->w[y][x];
    if (valid(xw))
        b->e[y][xw] += b->e[y][x];

    // ---- Diagonal ----
    int xse = x + b->se[y][x];
    int yse = y + b->se[y][x];
    if (valid(xse) && valid(yse))
        b->nw[yse][xse] += b->nw[y][x];
    int xnw = x - b->nw[y][x];
    int ynw = y - b->nw[y][x];
    if (valid(xnw) && valid(ynw))
        b->se[ynw][xnw] += b->se[y][x];

    // ---- Anti-diagonal ----
    int xne = x + b->ne[y][x];
    int yne = y - b->ne[y][x];
    if (valid(xne) && valid(yne))
        b->sw[yne][xne] += b->sw[y][x];
    int xsw = x - b->sw[y][x];
    int ysw = y + b->sw[y][x];
    if (valid(xsw) && valid(ysw))
        b->ne[ysw][xsw] += b->ne[y][x];

    return false;
}

unsigned int xor_shift()
{
    // Xorshift
    static unsigned long x = 1729163UL;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    x &= 0xffffffffU;
    return x;
}

void shuffle(int *a, int n)
{
    // Fisher-Yates shuffle
    for (int i = n - 1; i > 0; i--)
    {
        int k = xor_shift() % (i + 1);
        int temp = a[k];
        a[k] = a[i];
        a[i] = temp;
    }
}

int do_game()
{
    int free_cells[BOARD_SIZE_SQUARED];
    for (int i = 0; i < BOARD_SIZE_SQUARED; i++)
        free_cells[i] = i;

    shuffle(free_cells, BOARD_SIZE_SQUARED);

    Board circle, cross;
    board_init(&circle);
    board_init(&cross);

    for (int i = 0; i < BOARD_SIZE_SQUARED; i++)
    {
        int x = free_cells[i] % BOARD_SIZE;
        int y = free_cells[i] / BOARD_SIZE;

        if (i % 2 == 0)
        {
            if (board_update(&circle, x, y))
                return 0;
        }
        else
        {
            if (board_update(&cross, x, y))
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
        switch (res)
        {
        case 0:
            o++;
            break;
        case 1:
            x++;
            break;
        case -1:
            draw++;
            break;
        }
    }

    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("O/X/Draw: %d/%d/%d\n", o, x, draw);
    printf("Time taken: %d ms\n", (int)(time_taken * 1000));
    return 0;
}
