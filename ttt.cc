#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>
#include <chrono>

const size_t BOARD_SIZE = 20;
const size_t BOARD_SIZE_SQUARED = BOARD_SIZE * BOARD_SIZE;
const size_t WIN_CONDITION = 10;

class Board
{
    using BoardArray = std::array<std::array<bool, BOARD_SIZE>, BOARD_SIZE>;
    using CountArray = std::array<long long, BOARD_SIZE>;
    using DiagArray = std::array<long long, 2 * BOARD_SIZE - 1>;

    BoardArray board{};
    CountArray cols{};
    CountArray rows{};
    DiagArray diag{};
    DiagArray anti{};

public:
    Board() = default;

    void update(size_t x, size_t y)
    {
        board[y][x] = true;
        cols[x]++;
        rows[y]++;
        diag[x - y + BOARD_SIZE - 1]++;
        anti[x + y]++;
    }

    bool check_win(size_t x, size_t y) const
    {
        long long acc;
        size_t xa, ya;

        // Column
        if (cols[x] >= WIN_CONDITION)
        {
            acc = 1;
            ya = y + 1;
            while (ya < BOARD_SIZE && board[ya++][x])
                if (++acc == WIN_CONDITION)
                    return true;
            ya = y - 1;
            while (ya < BOARD_SIZE && board[ya--][x])
                if (++acc == WIN_CONDITION)
                    return true;
        }

        // Row
        if (rows[y] >= WIN_CONDITION)
        {
            acc = 1;
            xa = x + 1;
            while (xa < BOARD_SIZE && board[y][xa++])
                if (++acc == WIN_CONDITION)
                    return true;
            xa = x - 1;
            while (xa < BOARD_SIZE && board[y][xa--])
                if (++acc == WIN_CONDITION)
                    return true;
        }

        // Diagonal
        if (diag[x - y + BOARD_SIZE - 1] >= WIN_CONDITION)
        {
            acc = 1;
            xa = x + 1;
            ya = y + 1;
            while (xa < BOARD_SIZE && ya < BOARD_SIZE && board[ya++][xa++])
                if (++acc == WIN_CONDITION)
                    return true;
            xa = x - 1;
            ya = y - 1;
            while (xa < BOARD_SIZE && ya < BOARD_SIZE && board[ya--][xa--])
                if (++acc == WIN_CONDITION)
                    return true;
        }

        // Anti-diagonal
        if (anti[x + y] >= WIN_CONDITION)
        {
            acc = 1;
            xa = x + 1;
            ya = y - 1;
            while (xa < BOARD_SIZE && ya < BOARD_SIZE && board[ya--][xa++])
                if (++acc == WIN_CONDITION)
                    return true;
            xa = x - 1;
            ya = y + 1;
            while (xa < BOARD_SIZE && ya < BOARD_SIZE && board[ya++][xa--])
                if (++acc == WIN_CONDITION)
                    return true;
        }

        return false;
    }
};

class Game
{

    std::mt19937 rng_{1729};

public:
    enum class Result
    {
        Circle,
        Cross,
        Draw
    };

    Game() = default;

    Result play()
    {
        std::vector<size_t> free_cells(BOARD_SIZE_SQUARED);
        std::iota(free_cells.begin(), free_cells.end(), 0);
        std::shuffle(free_cells.begin(), free_cells.end(), rng_);

        Board circle, cross;

        for (size_t i = 0; i < BOARD_SIZE_SQUARED; ++i)
        {
            size_t x = free_cells[i] % BOARD_SIZE;
            size_t y = free_cells[i] / BOARD_SIZE;

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
};

int main()
{
    Game game;
    constexpr size_t n = 10000;
    size_t o = 0, x = 0, draw = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < n; ++i)
    {
        switch (game.play())
        {
        case Game::Result::Circle:
            o++;
            break;
        case Game::Result::Cross:
            x++;
            break;
        case Game::Result::Draw:
            draw++;
            break;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    printf("O/X/Draw: %zu/%zu/%zu\n", o, x, draw);
    printf("Time taken: %zu ms\n", duration.count());
    return 0;
}
