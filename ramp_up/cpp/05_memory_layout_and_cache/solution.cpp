// 05_memory_layout_and_cache — reference solution.
//
// Compile & run standalone:
//   clang++ -std=c++17 -Wall -o /tmp/mem solution.cpp && /tmp/mem
// For honest timings, let the optimizer in:
//   clang++ -std=c++17 -O2 -Wall -o /tmp/mem solution.cpp && /tmp/mem
// Prints ALL TESTS PASSED when every assert holds. The [timing] lines are
// informational only — no assert ever depends on a measurement, so the tests
// cannot flake on a slow or busy machine.

#include <cassert>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <list>
#include <utility>
#include <vector>

// Stopwatch used by main() to PRINT timings. Correctness asserts never look
// at these numbers — timings vary machine to machine; results must not.
static double ms_since(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration<double, std::milli>(
               std::chrono::steady_clock::now() - start)
        .count();
}

// ---- Drill 1: struct padding ------------------------------------------------
// Same four fields, two declaration orders. Each field must sit at an offset
// divisible by its alignment (double: 8, int: 4, char: 1), the compiler may
// NOT reorder fields for you — it can only insert invisible padding bytes —
// and the total size rounds up to a multiple of the largest field alignment.
// JAVA: the JVM reorders fields behind your back (plus a 12-16 byte object
//       header you never see), so declaration order never mattered. Here it does.
struct BadOrder {   // 1 + 7 pad + 8 + 1 + 3 pad + 4 = 24 bytes
    char   ready;
    double timestamp;
    char   flags;
    int    id;
};

struct GoodOrder {  // 8 + 4 + 1 + 1 + 2 tail pad = 16 bytes — same data, -33%
    double timestamp;
    int    id;
    char   ready;
    char   flags;
};

// Returns {sizeof(BadOrder), sizeof(GoodOrder)}. The interview move is
// predicting both numbers on paper BEFORE the compiler tells you.
std::pair<std::size_t, std::size_t> padded_size_report() {
    return {sizeof(BadOrder), sizeof(GoodOrder)};
}

// ---- Drill 2: traversal order over a row-major matrix -----------------------
// The matrix is ONE flat vector; element (r, c) lives at index r * cols + c —
// row-major: each row's floats sit side by side in memory.
// JAVA: float[][] is an array of POINTERS to separately heap-allocated row
//       objects. A flat float[rows*cols] with manual indexing is the C++ way,
//       and it is how images and tensors actually arrive.
// Row-by-row touches addresses in increasing order: every byte of every cache
// line gets used and the prefetcher streams the next lines ahead of the loop.
double sum_rows_first(const std::vector<float>& m, std::size_t rows,
                      std::size_t cols) {
    double total = 0.0;
    for (std::size_t r = 0; r < rows; ++r) {
        for (std::size_t c = 0; c < cols; ++c) {
            total += m[r * cols + c];
        }
    }
    return total;
}

// Same elements, same count of additions, same Big-O — but the inner loop now
// hops `cols * 4` bytes per step, using 4 bytes of each cache line it drags in.
double sum_cols_first(const std::vector<float>& m, std::size_t rows,
                      std::size_t cols) {
    double total = 0.0;
    for (std::size_t c = 0; c < cols; ++c) {
        for (std::size_t r = 0; r < rows; ++r) {
            total += m[r * cols + c];
        }
    }
    return total;
}

// ---- Drill 3: contiguous vs pointer-chasing ---------------------------------
// std::vector<int>: the ints themselves, back to back. One cache line holds
// 16-32 of them; the prefetcher sees the sequential walk and streams ahead.
// JAVA: even ArrayList<Integer> pointer-chases — it is an array of references
//       to boxed Integer objects scattered on the heap. vector<int> is what
//       you always wished ArrayList was.
long long sum_vector(const std::vector<int>& values) {
    long long total = 0;
    for (int v : values) {
        total += v;
    }
    return total;
}

// std::list<int>: a doubly-linked list — every element is its own heap
// allocation holding {prev, next, value} (~24 bytes to store 4). Each ++it is
// a dependent pointer load: the CPU cannot fetch node N+1 until node N arrives.
// JAVA: LinkedList<Integer>, and the same advice applies: almost never.
long long sum_list(const std::list<int>& values) {
    long long total = 0;
    for (int v : values) {
        total += v;
    }
    return total;
}

// ---- Drill 4: AoS vs SoA ----------------------------------------------------
// One detected object from a vision model. Six 4-byte fields, alignment 4:
// sizeof(Detection) == 24 with zero padding — check it with the drill above.
struct Detection {
    float x, y, w, h;   // box center + size, pixels
    float score;        // confidence in [0, 1]
    int   class_id;
};

// AoS (array of structs): vector<Detection>. Finding the best score reads 4
// useful bytes out of every 24 — the box fields ride through the cache as
// dead weight, and the stride-24 walk cannot use SIMD.
// Returns -1.0f for an empty frame (real scores are >= 0).
float top_score_aos(const std::vector<Detection>& dets) {
    float best = -1.0f;
    for (const Detection& d : dets) {
        if (d.score > best) {
            best = d.score;
        }
    }
    return best;
}

// SoA (struct of arrays): the scores alone, packed. Same answer, 1/6 the
// memory traffic, and a layout SIMD can eat 4-16 floats at a time.
float top_score_soa(const std::vector<float>& scores) {
    float best = -1.0f;
    for (float s : scores) {
        if (s > best) {
            best = s;
        }
    }
    return best;
}

// ---- Drill 5: reserve() and reallocation ------------------------------------
// Counts how many times the vector's capacity CHANGES while building it: each
// change is a reallocation — allocate a bigger block, copy every element,
// free the old block (and every pointer/iterator into it dies).
// JAVA: ArrayList grows ~1.5x per overflow; ensureCapacity(n) is reserve(n).
// Measured on this machine's libc++: reserve(n) jumps capacity 0 -> n in one
// step and the n push_backs never touch it again, so this returns exactly 1.
int fill_with_reserve(std::size_t n) {
    std::vector<int> v;
    std::size_t cap = v.capacity();  // a fresh vector owns no memory: cap == 0
    int changes = 0;
    v.reserve(n);                    // ONE allocation, up front
    if (v.capacity() != cap) {
        ++changes;
        cap = v.capacity();
    }
    for (std::size_t i = 0; i < n; ++i) {
        v.push_back(static_cast<int>(i));
        if (v.capacity() != cap) {
            ++changes;
            cap = v.capacity();
        }
    }
    return changes;
}

// Same fill without the reserve: capacity crawls 1, 2, 4, ... doubling every
// time it overflows — 21 reallocations for a million elements on this libc++,
// and every one of them recopies everything pushed so far.
int fill_without_reserve(std::size_t n) {
    std::vector<int> v;
    std::size_t cap = v.capacity();
    int changes = 0;
    for (std::size_t i = 0; i < n; ++i) {
        v.push_back(static_cast<int>(i));
        if (v.capacity() != cap) {
            ++changes;
            cap = v.capacity();
        }
    }
    return changes;
}

int main() {
    std::cout << std::fixed << std::setprecision(1);

    // padded_size_report
    {
        std::pair<std::size_t, std::size_t> report = padded_size_report();
        assert(report.first == sizeof(BadOrder));
        assert(report.second == sizeof(GoodOrder));
        // The four fields alone need 14 bytes; only padding explains more.
        assert(report.first >= 14 && report.second >= 14);
        // The whole point: reordering the SAME fields genuinely shrank it.
        assert(report.second < report.first);
        std::cout << "[layout] BadOrder = " << report.first
                  << " bytes, GoodOrder = " << report.second
                  << " bytes (same four fields)\n";
        assert(sizeof(Detection) == 6 * 4);  // six 4-byte fields, no padding
    }

    // sum_rows_first / sum_cols_first
    {
        // Small exact case: 3x4 matrix holding 1..12, both orders must see 78.
        // Integer-valued floats accumulate EXACTLY in a double (every value
        // and partial sum is a smallish integer), so == is safe here.
        std::vector<float> small(12);
        for (std::size_t i = 0; i < 12; ++i) {
            small[i] = static_cast<float>(i + 1);
        }
        assert(sum_rows_first(small, 3, 4) == 78.0);
        assert(sum_cols_first(small, 3, 4) == 78.0);

        const std::size_t R = 2048, C = 2048;  // 4M floats = 16 MB
        std::vector<float> m(R * C);
        for (std::size_t i = 0; i < m.size(); ++i) {
            m[i] = static_cast<float>(i % 7);  // integer-valued: exact sums
        }
        auto t0 = std::chrono::steady_clock::now();
        double rows = sum_rows_first(m, R, C);
        double rows_ms = ms_since(t0);
        t0 = std::chrono::steady_clock::now();
        double cols = sum_cols_first(m, R, C);
        double cols_ms = ms_since(t0);
        assert(rows == cols);  // same elements => identical sum, either order
        std::cout << "[timing] 2048x2048 sum: rows-first " << rows_ms
                  << " ms, cols-first " << cols_ms << " ms\n";
    }

    // sum_vector / sum_list
    {
        assert(sum_vector({1, 2, 3}) == 6);
        assert(sum_list({1, 2, 3}) == 6);
        assert(sum_vector({}) == 0);

        const std::size_t N = 2'000'000;
        std::vector<int> vec(N);
        for (std::size_t i = 0; i < N; ++i) {
            vec[i] = static_cast<int>(i % 256);
        }
        std::list<int> lst(vec.begin(), vec.end());
        auto t0 = std::chrono::steady_clock::now();
        long long sv = sum_vector(vec);
        double vec_ms = ms_since(t0);
        t0 = std::chrono::steady_clock::now();
        long long sl = sum_list(lst);
        double list_ms = ms_since(t0);
        assert(sv == sl);
        assert(sv > 0);  // a stub returning 0 must not sneak past
        std::cout << "[timing] 2M ints: vector " << vec_ms << " ms, list "
                  << list_ms << " ms\n";
    }

    // top_score_aos / top_score_soa
    {
        std::vector<Detection> tiny = {
            {10.f, 20.f, 5.f, 5.f, 0.30f, 3},
            {40.f, 25.f, 9.f, 7.f, 0.90f, 0},
            {12.f, 80.f, 4.f, 6.f, 0.75f, 3},
        };
        assert(top_score_aos(tiny) == 0.90f);
        assert(top_score_soa({0.30f, 0.90f, 0.75f}) == 0.90f);
        assert(top_score_aos({}) == -1.0f);  // empty frame sentinel
        assert(top_score_soa({}) == -1.0f);

        const std::size_t N = 2'000'000;
        std::vector<Detection> aos(N);
        std::vector<float> scores(N);
        for (std::size_t i = 0; i < N; ++i) {
            float s = static_cast<float>((i * 2654435761u) % 1000) / 1000.0f;
            aos[i] = {1.f, 2.f, 3.f, 4.f, s, static_cast<int>(i % 80)};
            scores[i] = s;
        }
        aos[N / 2].score = 1.5f;  // plant one exact, known winner
        scores[N / 2] = 1.5f;
        auto t0 = std::chrono::steady_clock::now();
        float best_aos = top_score_aos(aos);
        double aos_ms = ms_since(t0);
        t0 = std::chrono::steady_clock::now();
        float best_soa = top_score_soa(scores);
        double soa_ms = ms_since(t0);
        assert(best_aos == 1.5f);
        assert(best_soa == 1.5f);
        assert(best_aos == best_soa);  // layout must never change the answer
        std::cout << "[timing] 2M detections, top score: AoS " << aos_ms
                  << " ms, SoA " << soa_ms << " ms\n";
    }

    // fill_with_reserve / fill_without_reserve
    {
        // Verified empirically on this libc++: reserve makes capacity change
        // exactly ONCE (0 -> n); without it, growth doubles over and over.
        assert(fill_with_reserve(1) == 1);
        assert(fill_with_reserve(100'000) == 1);
        int changes = fill_without_reserve(100'000);
        assert(changes > 1);
        assert(changes > fill_with_reserve(100'000));

        const std::size_t N = 2'000'000;
        auto t0 = std::chrono::steady_clock::now();
        int without = fill_without_reserve(N);
        double no_res_ms = ms_since(t0);
        t0 = std::chrono::steady_clock::now();
        int with = fill_with_reserve(N);
        double res_ms = ms_since(t0);
        std::cout << "[timing] 2M push_backs: no reserve " << no_res_ms
                  << " ms (" << without << " reallocations), reserve "
                  << res_ms << " ms (" << with << " allocation)\n";
    }

    std::cout << "ALL TESTS PASSED" << std::endl;
    return 0;
}
