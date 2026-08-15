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

#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>

constexpr int BOARD_SIZE = 20;
constexpr int BOARD_PADDED = BOARD_SIZE + 2;
constexpr int BOARD_CELLS = BOARD_PADDED * BOARD_PADDED;
constexpr int BOARD_SIZE_SQUARED = BOARD_SIZE * BOARD_SIZE;
constexpr int WIN_CONDITION = 10;

// Neighbour deltas on the flattened (y * BOARD_PADDED + x) board.
constexpr int D_S = BOARD_PADDED;
constexpr int D_E = 1;
constexpr int D_SE = BOARD_PADDED + 1;
constexpr int D_SW = BOARD_PADDED - 1;

struct Cell
{
    uint8_t s, n, e, w, se, nw, ne, sw;
};

static_assert(sizeof(Cell) == 8, "Cell must pack into a single 64-bit word");

// The same deltas measured in bytes, for direct addressing into the board.
constexpr intptr_t BYTES_S = D_S * intptr_t(sizeof(Cell));
constexpr intptr_t BYTES_E = D_E * intptr_t(sizeof(Cell));
constexpr intptr_t BYTES_SE = D_SE * intptr_t(sizeof(Cell));
constexpr intptr_t BYTES_SW = D_SW * intptr_t(sizeof(Cell));

class Board
{
    Cell b[BOARD_CELLS];

public:
    void clear() { std::memset(b, 0, sizeof(b)); }
    bool check_win(int p);
};

inline bool Board::check_win(int p)
{
    Cell &q = b[p];
    uint8_t *base = (uint8_t *)&q;

    // One direction at a time. A winning move ends the game and the board
    // is cleared before it is used again, so the endpoint writes of the
    // earlier directions are dead and it does not matter that they have
    // already happened -- and keeping only one pair of runs live at a time
    // is what stops the register allocator from spilling. None of the
    // eight targets can be this cell (every offset is at least one step),
    // so a store never disturbs a run length still to be read.
    intptr_t s = q.s, n = q.n;
    intptr_t col = s + n + 1;
    if (col >= WIN_CONDITION)
        return true;
    base[-n * BYTES_S - BYTES_S + 0] = uint8_t(col);
    base[+s * BYTES_S + BYTES_S + 1] = uint8_t(col);

    intptr_t e = q.e, w = q.w;
    intptr_t row = e + w + 1;
    if (row >= WIN_CONDITION)
        return true;
    base[-w * BYTES_E - BYTES_E + 2] = uint8_t(row);
    base[+e * BYTES_E + BYTES_E + 3] = uint8_t(row);

    intptr_t se = q.se, nw = q.nw;
    intptr_t diag = se + nw + 1;
    if (diag >= WIN_CONDITION)
        return true;
    base[-nw * BYTES_SE - BYTES_SE + 4] = uint8_t(diag);
    base[+se * BYTES_SE + BYTES_SE + 5] = uint8_t(diag);

    intptr_t ne = q.ne, sw = q.sw;
    intptr_t anti = ne + sw + 1;
    if (anti >= WIN_CONDITION)
        return true;
    base[+sw * BYTES_SW + BYTES_SW + 6] = uint8_t(anti);
    base[-ne * BYTES_SW - BYTES_SW + 7] = uint8_t(anti);

    return false;
}

enum class Result
{
    Circle,
    Cross,
    Draw
};

// Playable positions in scan order, pre-flattened to y * BOARD_PADDED + x.
struct PosTable
{
    uint16_t v[BOARD_SIZE_SQUARED];
};

constexpr PosTable make_positions()
{
    PosTable t = {};
    int i = 0;
    for (int y = 1; y <= BOARD_SIZE; y++)
        for (int x = 1; x <= BOARD_SIZE; x++)
            t.v[i++] = uint16_t(y * BOARD_PADDED + x);
    return t;
}

constexpr PosTable POS = make_positions();

static uint64_t rng_state = 1729163UL;
static Board circle, cross;

// How many shuffle steps and moves each pass of the fused loop handles.
// Two is the sweet spot here: enough independent work to cover the xorshift
// latency, few enough live values to keep the register allocator honest.
constexpr int CHUNK = 2;
static_assert(CHUNK % 2 == 0 && BOARD_SIZE_SQUARED % CHUNK == 0,
              "CHUNK must be even and divide the board");

// One inside-out Fisher-Yates step. The xorshift keeps a 64-bit state for
// the first two rounds (the high bits feed the >> 17) but finishes in 32
// bits, which folds the 0xffffffff mask into the truncation and takes a
// cycle off the loop-carried dependency. `m` is the 1-based step counter,
// so the multiplier needs no separate increment.
#define SHUFFLE_STEP(rng, m, out, pos)                         \
    do                                                         \
    {                                                          \
        uint64_t t_ = (rng) ^ ((rng) << 13);                   \
        uint32_t u_ = uint32_t(t_) ^ uint32_t(t_ >> 17);       \
        (rng) = u_ ^ (u_ << 5);                                \
        int j_ = int((rng) * uint64_t(m) >> 32);               \
        (out)[(m) -1] = (out)[j_];                             \
        (out)[j_] = (pos)[(m) -1];                             \
        (m)++;                                                 \
    } while (0)

// Plays the permutation in `pos` while building the *next* game's
// permutation in `out`. The shuffle is a serial xorshift chain and on its
// own runs latency-bound; interleaving it with the move loop, which is
// throughput-bound and independent of it, hides almost all of it. The RNG
// is still consumed in exactly the original order, one whole permutation
// at a time, so the games are unchanged.
Result play(const uint16_t *__restrict pos, uint16_t *__restrict out)
{
    circle.clear();
    cross.clear();

    const uint16_t *__restrict pt = POS.v;
    uint64_t rng = rng_state;
    int m = 1;
    Result r = Result::Draw;

    for (int k = 0; k < BOARD_SIZE_SQUARED; k += CHUNK)
    {
        for (int c = 0; c < CHUNK; c++)
        {
            SHUFFLE_STEP(rng, m, out, pt);
        }

        for (int c = 0; c < CHUNK; c += 2)
        {
            if (circle.check_win(pos[k + c]))
            {
                r = Result::Circle;
                goto done;
            }
            if (cross.check_win(pos[k + c + 1]))
            {
                r = Result::Cross;
                goto done;
            }
        }
    }
done:
    // A game that ended early still owes the rest of the permutation.
    while (m <= BOARD_SIZE_SQUARED)
    {
        SHUFFLE_STEP(rng, m, out, pt);
    }

    rng_state = rng;
    return r;
}

int main()
{
    constexpr int n = 10000;
    int o = 0, x = 0, draw = 0;

    static uint16_t perm[2][BOARD_SIZE_SQUARED];

    auto start = std::chrono::high_resolution_clock::now();

    // Prime the pipeline with the first game's permutation.
    {
        uint64_t rng = rng_state;
        int m = 1;
        while (m <= BOARD_SIZE_SQUARED)
        {
            SHUFFLE_STEP(rng, m, perm[0], POS.v);
        }
        rng_state = rng;
    }

    for (int i = 0; i < n; i++)
    {
        switch (play(perm[i & 1], perm[~i & 1]))
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
