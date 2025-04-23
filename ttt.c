// Credit: Jeremy Chen (gh:intgrah)

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define BOARD_SIZE 20
#define BOARD_SIZE_SQUARED (BOARD_SIZE * BOARD_SIZE)
#define WIN_CONDITION 10

typedef bool board[BOARD_SIZE][BOARD_SIZE];

bool check_win(int x, int y, board b, int *cols, int *rows)
{
    int acc, xa, ya;

    // Column
    if (cols[x] >= WIN_CONDITION)
    {
        acc = 1;
        ya = y + 1;
        while (ya < BOARD_SIZE && b[ya++][x])
            if (++acc == WIN_CONDITION)
                return true;
        ya = y - 1;
        while (0 <= ya && b[ya--][x])
            if (++acc == WIN_CONDITION)
                return true;
    }

    // Row
    if (rows[y] >= WIN_CONDITION)
    {
        acc = 1;
        xa = x + 1;
        while (xa < BOARD_SIZE && b[y][xa++])
            if (++acc == WIN_CONDITION)
                return true;
        xa = x - 1;
        while (0 <= xa && b[y][xa--])
            if (++acc == WIN_CONDITION)
                return true;
    }

    // Diagonal
    if (BOARD_SIZE - abs(x - y) >= WIN_CONDITION)
    {
        acc = 1;
        xa = x + 1;
        ya = y + 1;
        while (xa < BOARD_SIZE && ya < BOARD_SIZE && b[ya++][xa++])
            if (++acc == WIN_CONDITION)
                return true;
        xa = x - 1;
        ya = y - 1;
        while (0 <= xa && 0 <= ya && b[ya--][xa--])
            if (++acc == WIN_CONDITION)
                return true;
    }

    // Anti-diagonal
    if (BOARD_SIZE - abs(x + y - BOARD_SIZE + 1) >= WIN_CONDITION)
    {
        acc = 1;
        xa = x + 1;
        ya = y - 1;
        while (xa < BOARD_SIZE && 0 <= ya && b[ya--][xa++])
            if (++acc == WIN_CONDITION)
                return true;
        xa = x - 1;
        ya = y + 1;
        while (0 <= xa && ya < BOARD_SIZE && b[ya++][xa--])
            if (++acc == WIN_CONDITION)
                return true;
    }

    return false;
}

void shuffle(int *a, int n)
{
    // Fisher-Yates shuffle
    for (int i = n - 1; i > 0; i--)
    {
        int k = rand() % (i + 1);
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

    int ocols[BOARD_SIZE] = {0};
    int orows[BOARD_SIZE] = {0};
    int xcols[BOARD_SIZE] = {0};
    int xrows[BOARD_SIZE] = {0};
    board os = {false};
    board xs = {false};

    for (int i = 0; i < BOARD_SIZE_SQUARED; i++)
    {
        int x = free_cells[i] % BOARD_SIZE;
        int y = free_cells[i] / BOARD_SIZE;
        if (i % 2 == 0)
        {
            os[y][x] = true;

            if (check_win(x, y, os, ocols, orows))
                return 0;
        }
        else
        {
            xs[y][x] = true;

            if (check_win(x, y, xs, xcols, xrows))
                return 1;
        }
    }
    return -1;
}

int main()
{
    srand(1729);

    // Debug: Print first 10 random numbers to verify sequence
    printf("First 10 random numbers:\n");
    for (int i = 0; i < 10; i++)
    {
        printf("%d ", rand());
    }
    printf("\n");

    int n = 10000;
    int o = 0, x = 0, draw = 0;

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

    printf("O/X/Draw: %d/%d/%d\n", o, x, draw);
    return 0;
}
