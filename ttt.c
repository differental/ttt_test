// Credit: Jeremy Chen (gh:intgrah)

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define BOARD_SIZE 20
#define BOARD_SIZE_SQUARED (BOARD_SIZE * BOARD_SIZE)
#define WIN_CONDITION 10

typedef enum
{
    NONE = 0,
    CIRCLE = 1,
    CROSS = 2
} cell;
typedef cell board[BOARD_SIZE][BOARD_SIZE];

bool check_line(cell p, int n, int dx, int dy, int x, int y, board* b)
{
    for (int acc = 0; 0 < n; n--, x += dx, y += dy)
    {
        if ((*b)[y][x] == p)
        {
            acc++;
            if (acc == WIN_CONDITION)
                return true;
        }
        else
        {
            acc = 0;
            if (n - 1 < WIN_CONDITION)
                return false;
        }
    }
    return false;
}

bool check_win(cell p, int x, int y, board* b)
{
    // Column
    if (check_line(p, BOARD_SIZE, 0, 1, x, 0, b))
        return true;
    // Row
    if (check_line(p, BOARD_SIZE, 1, 0, 0, y, b))
        return true;
    // Diagonal
    int diag_len = BOARD_SIZE - abs(x - y);
    int d = (x < y) ? x : y;
    if (diag_len >= WIN_CONDITION && check_line(p, diag_len, 1, 1, x - d, y - d, b))
        return true;
    // Anti-diagonal
    diag_len = BOARD_SIZE - abs(x + y - BOARD_SIZE + 1);
    d = ((BOARD_SIZE - 1 - x) < y) ? (BOARD_SIZE - 1 - x) : y;
    if (diag_len >= WIN_CONDITION && check_line(p, diag_len, -1, 1, x + d, y - d, b))
        return true;
    return false;
}

void shuffle(int *a)
{
    // Fisher-Yates shuffle
    for (int i = BOARD_SIZE_SQUARED - 1; i > 0; i--)
    {
        int k = rand() % (i + 1);
        int temp = a[k];
        a[k] = a[i];
        a[i] = temp;
    }
}

int free_cells[BOARD_SIZE_SQUARED];
cell do_game(void)
{
    board b = {NONE};
    int nn = BOARD_SIZE_SQUARED;
    for (int i = 0; i < nn; i++)
        free_cells[i] = i;

    shuffle(free_cells);

    cell res = NONE;
    for (int i = 0; i < nn; i++)
    {
        int x = free_cells[i] / BOARD_SIZE;
        int y = free_cells[i] % BOARD_SIZE;
        cell p = (i % 2 == 0) ? CIRCLE : CROSS;
        b[y][x] = p;

        if (check_win(p, x, y, &b))
            return p;
    }
    return NONE;
}

int main(void)
{
    srand(2134);
    int n = 100000;
    int o = 0, x = 0, draw = 0;

    for (int i = 0; i < n; i++)
    {
        switch (do_game())
        {
        case CIRCLE:
            o++;
            break;
        case CROSS:
            x++;
            break;
        case NONE:
            draw++;
            break;
        }
    }

    printf("O/X/Draw: %d/%d/%d\n", o, x, draw);
    return 0;
}
