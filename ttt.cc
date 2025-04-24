#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

const int BOARD_SIZE = 20;
const int BOARD_SIZE_SQUARED = BOARD_SIZE * BOARD_SIZE;
const int WIN_CONDITION = 10;

class Board
{
    using BoardArray = std::array<std::array<bool, BOARD_SIZE>, BOARD_SIZE>;
    using CountArray = std::array<int, BOARD_SIZE>;
    using DiagArray = std::array<int, 2 * BOARD_SIZE - 1>;

    BoardArray board{};
    CountArray cols{};
    CountArray rows{};
    DiagArray diag{};
    DiagArray anti{};

public:
    Board() = default;

    void update(int x, int y)
    {
        board[y][x] = true;
        cols[x]++;
        rows[y]++;
        diag[x - y + BOARD_SIZE - 1]++;
        anti[x + y]++;
    }

    bool check_win(int x, int y) const
    {
        // Column
        if (cols[x] >= WIN_CONDITION)
        {
            int acc = 1;
            for (int ya = y; ++ya < BOARD_SIZE && board[ya][x]; acc++)
                ;
            for (int ya = y; 0 <= --ya && board[ya][x]; acc++)
                ;
            if (acc >= WIN_CONDITION)
                return true;
        }

        // Row
        if (rows[y] >= WIN_CONDITION)
        {
            int acc = 1;
            for (int xa = x; ++xa < BOARD_SIZE && board[y][xa]; acc++)
                ;
            for (int xa = x; 0 <= --xa && board[y][xa]; acc++)
                ;
            if (acc >= WIN_CONDITION)
                return true;
        }

        // Diagonal
        if (diag[x - y + BOARD_SIZE - 1] >= WIN_CONDITION)
        {
            int acc = 1;
            for (int xa = x, ya = y; ++xa < BOARD_SIZE && ++ya < BOARD_SIZE && board[ya][xa]; acc++)
                ;
            for (int xa = x, ya = y; 0 <= --xa && 0 <= --ya && board[ya][xa]; acc++)
                ;
            if (acc >= WIN_CONDITION)
                return true;
        }

        // Anti-diagonal
        if (anti[x + y] >= WIN_CONDITION)
        {
            int acc = 1;
            for (int xa = x, ya = y; ++xa < BOARD_SIZE && 0 <= --ya && board[ya][xa]; acc++)
                ;
            for (int xa = x, ya = y; 0 <= --xa && ++ya < BOARD_SIZE && board[ya][xa]; acc++)
                ;
            if (acc >= WIN_CONDITION)
                return true;
        }

        return false;
    }
};

enum class Result
{
    Circle,
    Cross,
    Draw
};

Result do_game(std::mt19937 &rng)
{
    std::vector<int> free_cells(BOARD_SIZE_SQUARED);
    std::iota(free_cells.begin(), free_cells.end(), 0);
    std::shuffle(free_cells.begin(), free_cells.end(), rng);

    Board circle, cross;

    for (int i = 0; i < BOARD_SIZE_SQUARED; ++i)
    {
        int x = free_cells[i] % BOARD_SIZE;
        int y = free_cells[i] / BOARD_SIZE;

        if (i % 2 == 0)
        {
            circle.update(x, y);
            if (circle.check_win(x, y))
                return Result::Circle;
        }
        else
        {
            cross.update(x, y);
            if (cross.check_win(x, y))
                return Result::Cross;
        }
    }

    return Result::Draw;
}

int main()
{
    std::mt19937 rng{1729};
    constexpr int n = 10000;
    int o = 0, x = 0, draw = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < n; i++)
    {
        switch (do_game(rng))
        {
        case Result::Circle:
            o++;
            break;
        case Result::Cross:
            x++;
            break;
        case Result::Draw:
            draw++;
            break;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "O/X/Draw: " << o << "/" << x << "/" << draw << std::endl;
    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
    return 0;
}
