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

#include <array>
#include <chrono>
#include <iostream>

constexpr int BOARD_SIZE = 20;
constexpr int BOARD_PADDED = BOARD_SIZE + 2;
constexpr int BOARD_SIZE_SQUARED = BOARD_SIZE * BOARD_SIZE;
constexpr int WIN_CONDITION = 10;

struct Cell
{
    uint8_t s, n, e, w, se, nw, ne, sw;
};

class Board
{
    Cell b[BOARD_PADDED][BOARD_PADDED] = {};

public:
    bool check_win(int x, int y);
};

bool Board::check_win(int x, int y)
{
    Cell q = b[y][x];
    uint8_t col = q.s + 1 + q.n;
    uint8_t row = q.w + 1 + q.e;
    uint8_t diag = q.nw + 1 + q.se;
    uint8_t anti = q.ne + 1 + q.sw;

    if (col >= WIN_CONDITION || row >= WIN_CONDITION || diag >= WIN_CONDITION || anti >= WIN_CONDITION)
    {
        return true;
    }

    b[y + q.s + 1][x].n = b[y - q.n - 1][x].s = col;
    b[y][x + q.e + 1].w = b[y][x - q.w - 1].e = row;
    b[y + q.se + 1][x + q.se + 1].nw = b[y - q.nw - 1][x - q.nw - 1].se = diag;
    b[y - q.ne - 1][x + q.ne + 1].sw = b[y + q.sw + 1][x - q.sw - 1].ne = anti;

    return false;
}

enum class Result
{
    Circle,
    Cross,
    Draw
};

uint32_t xorshift(uint32_t max)
{
    // Xorshift
    static uint64_t x = 1729163UL;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    x &= 0xffffffffU;
    return x * uint64_t(max) >> 32;
}

Result do_game()
{
    std::array<uint8_t, BOARD_SIZE_SQUARED> free_x;
    std::array<uint8_t, BOARD_SIZE_SQUARED> free_y;

    for (int y = 1, i = 0; y <= BOARD_SIZE; y++)
        for (int x = 1; x <= BOARD_SIZE; x++, i++)
        {
            int j = xorshift(i + 1);
            free_x[i] = free_x[j];
            free_y[i] = free_y[j];
            free_x[j] = x;
            free_y[j] = y;
        }

    Board circle, cross;

    for (int i = 0; i < BOARD_SIZE_SQUARED; i++)
    {
        int x = free_x[i];
        int y = free_y[i];

        if (i % 2 == 0)
        {
            if (circle.check_win(x, y))
                return Result::Circle;
        }
        else
        {
            if (cross.check_win(x, y))
                return Result::Cross;
        }
    }
    return Result::Draw;
}

int main()
{
    constexpr int n = 10000;
    int o = 0, x = 0, draw = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < n; i++)
    {
        switch (do_game())
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
