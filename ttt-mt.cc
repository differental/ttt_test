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

// Multi-threaded companion to ttt.cc. Same simulation, same output; the
// games are dealt out one worker per core.
//
// The games look strictly sequential because the xorshift state carries
// from one into the next, but the step function is linear over GF(2):
// every operation in it is a shift or an xor, and the 32-bit mask is a
// projection. So one step is a 32x32 bit-matrix M, its 400th power J is
// the state change across a whole game's draws, and applying J to a seed
// skips a game without simulating it. Each game therefore gets an
// independently computable starting state, and the three tallies are
// sums, so the order they are accumulated in does not matter. The output
// is bit-identical to the serial program.
//
// This is deliberately NOT part of `make bench`: every other language in
// this repo runs single-threaded (the Go entry is pinned to CPUS=1), so
// timing this alongside them would not be a like-for-like comparison.

#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

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

// ---------------------------------------------------------------- jump-ahead

constexpr uint32_t SEED = 1729163UL;

static inline uint32_t rng_step(uint32_t x)
{
    uint64_t t = uint64_t(x) ^ (uint64_t(x) << 13);
    uint32_t u = uint32_t(t) ^ uint32_t(t >> 17);
    return u ^ (u << 5);
}

// r[i] is the image of basis vector e_i, so `apply` is a matrix-vector
// product over GF(2) and `compose` composes the maps.
struct Mat
{
    uint32_t r[32];
};

static uint32_t apply(const Mat &m, uint32_t x)
{
    uint32_t y = 0;
    while (x)
    {
        y ^= m.r[__builtin_ctz(x)];
        x &= x - 1;
    }
    return y;
}

static Mat compose(const Mat &a, const Mat &b)
{
    Mat c;
    for (int i = 0; i < 32; i++)
        c.r[i] = apply(b, a.r[i]);
    return c;
}

// J = M^BOARD_SIZE_SQUARED, the state change across one game's draws.
static Mat game_jump()
{
    Mat m;
    for (int i = 0; i < 32; i++)
        m.r[i] = rng_step(1u << i);

    // Composed the plain way: 399 products of 32x32 bit-matrices, once, at
    // startup. Square-and-multiply would save microseconds and is easy to
    // get subtly wrong.
    Mat j = m;
    for (int i = 1; i < BOARD_SIZE_SQUARED; i++)
        j = compose(j, m);
    return j;
}

// ------------------------------------------------------------------ workers

struct Counts
{
    long o, x, d;
};

// The same fused loop as ttt.cc: play `pos` while building the next game's
// permutation into `out`, so the serial xorshift chain hides behind the
// throughput-bound move loop.
static Result play(Board &circle, Board &cross, const uint16_t *__restrict pos,
                   uint16_t *__restrict out, uint64_t rng)
{
    circle.clear();
    cross.clear();

    const uint16_t *__restrict pt = POS.v;
    int m = 1;
    Result r = Result::Draw;

    for (int k = 0; k < BOARD_SIZE_SQUARED; k += CHUNK)
    {
        for (int c = 0; c < CHUNK; c++)
            SHUFFLE_STEP(rng, m, out, pt);

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
    while (m <= BOARD_SIZE_SQUARED)
        SHUFFLE_STEP(rng, m, out, pt);

    return r;
}

static void worker(int lo, int hi, const uint32_t *seeds, Counts *out_counts)
{
    Counts c = {0, 0, 0};
    if (lo >= hi)
    {
        *out_counts = c;
        return;
    }

    Board circle, cross;
    uint16_t perm[2][BOARD_SIZE_SQUARED];

    // Prime this worker's pipeline with its first game's permutation.
    {
        uint64_t rng = seeds[lo];
        int m = 1;
        while (m <= BOARD_SIZE_SQUARED)
            SHUFFLE_STEP(rng, m, perm[0], POS.v);
    }

    for (int g = lo; g < hi; g++)
    {
        int slot = (g - lo) & 1;
        // The range's last game has no successor to shuffle for, so it
        // refills the spare buffer with its own seed and that is discarded.
        uint64_t next = (g + 1 < hi) ? seeds[g + 1] : seeds[lo];

        switch (play(circle, cross, perm[slot], perm[slot ^ 1], next))
        {
        case Result::Circle:
            c.o++;
            break;
        case Result::Cross:
            c.x++;
            break;
        case Result::Draw:
            c.d++;
            break;
        }
    }

    *out_counts = c;
}

int main()
{
    constexpr int n = 10000;

    auto start = std::chrono::high_resolution_clock::now();

    // Per-game starting states: 32 xors each, noise next to the 400 real
    // draws every game still performs.
    std::vector<uint32_t> seeds(n);
    {
        Mat j = game_jump();
        uint32_t s = SEED;
        for (int i = 0; i < n; i++)
        {
            seeds[i] = s;
            s = apply(j, s);
        }
    }

    unsigned t = std::thread::hardware_concurrency();
    if (t == 0)
        t = 1;
    if (t > unsigned(n))
        t = unsigned(n);

    std::vector<Counts> parts(t);
    std::vector<std::thread> pool;
    pool.reserve(t);
    for (unsigned i = 0; i < t; i++)
    {
        int lo = int((long long)n * i / t);
        int hi = int((long long)n * (i + 1) / t);
        pool.emplace_back(worker, lo, hi, seeds.data(), &parts[i]);
    }
    for (std::thread &th : pool)
        th.join();

    long o = 0, x = 0, draw = 0;
    for (const Counts &c : parts)
    {
        o += c.o;
        x += c.x;
        draw += c.d;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "O/X/Draw: " << o << "/" << x << "/" << draw << std::endl;
    std::cout << "Time taken: " << duration.count() << " ms"
              << " (" << t << " threads)" << std::endl;
    return 0;
}
