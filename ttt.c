// Credit: Jeremy Chen (gh:intgrah)

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define BOARD_SIZE 40
#define BOARD_SIZE_SQUARED (BOARD_SIZE * BOARD_SIZE)
#define WIN_CONDITION 12

typedef struct
{
    bool board[BOARD_SIZE][BOARD_SIZE];
    int cols[BOARD_SIZE];
    int rows[BOARD_SIZE];
    int diag[2 * BOARD_SIZE - 1];
    int anti[2 * BOARD_SIZE - 1];
} Board;

void board_init(Board *b)
{
    *b = (Board){
        .board = {{false}},
        .cols = {0},
        .rows = {0},
        .diag = {0},
        .anti = {0}};
}

void board_update(Board *b, int x, int y)
{
    b->board[y][x] = true;
    b->cols[x]++;
    b->rows[y]++;
    b->diag[x - y + BOARD_SIZE - 1]++;
    b->anti[x + y]++;
}

bool check_win(Board *b, int x, int y)
{
    // Column
    if (b->cols[x] >= WIN_CONDITION)
    {
        int acc = 1;
        for (int ya = y; ++ya < BOARD_SIZE && b->board[ya][x]; acc++)
            ;
        for (int ya = y; 0 <= --ya && b->board[ya][x]; acc++)
            ;
        if (acc >= WIN_CONDITION)
            return true;
    }

    // Row
    if (b->rows[y] >= WIN_CONDITION)
    {
        int acc = 1;
        for (int xa = x; ++xa < BOARD_SIZE && b->board[y][xa]; acc++)
            ;
        for (int xa = x; 0 <= --xa && b->board[y][xa]; acc++)
            ;
        if (acc >= WIN_CONDITION)
            return true;
    }

    // Diagonal
    if (b->diag[x - y + BOARD_SIZE - 1] >= WIN_CONDITION)
    {
        int acc = 1;
        for (int xa = x, ya = y; ++xa < BOARD_SIZE && ++ya < BOARD_SIZE && b->board[ya][xa]; acc++)
            ;
        for (int xa = x, ya = y; 0 <= --xa && 0 <= --ya && b->board[ya][xa]; acc++)
            ;
        if (acc >= WIN_CONDITION)
            return true;
    }

    // Anti-diagonal
    if (b->anti[x + y] >= WIN_CONDITION)
    {
        int acc = 1;
        for (int xa = x, ya = y; ++xa < BOARD_SIZE && 0 <= --ya && b->board[ya][xa]; acc++)
            ;
        for (int xa = x, ya = y; 0 <= --xa && ++ya < BOARD_SIZE && b->board[ya][xa]; acc++)
            ;
        if (acc >= WIN_CONDITION)
            return true;
    }

    return false;
}

unsigned int rand()
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

    Board circle, cross;
    board_init(&circle);
    board_init(&cross);

    for (int i = 0; i < BOARD_SIZE_SQUARED; i++)
    {
        int x = free_cells[i] % BOARD_SIZE;
        int y = free_cells[i] / BOARD_SIZE;

        if (i % 2 == 0)
        {
            board_update(&circle, x, y);
            if (check_win(&circle, x, y))
                return 0;
        }
        else
        {
            board_update(&cross, x, y);
            if (check_win(&cross, x, y))
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
