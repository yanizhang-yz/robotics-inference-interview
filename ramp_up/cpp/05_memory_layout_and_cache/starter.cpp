// 05_memory_layout_and_cache — YOUR attempt. Fill in the TODO bodies (and fix
// one struct's field order), then run:
//   PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache -v
// Or compile and run directly:
//   clang++ -std=c++17 -Wall -o /tmp/mem starter.cpp && /tmp/mem
// For honest timings, add -O2. The stubs compile as-is but fail main()'s
// asserts until you implement them. The [timing] lines are informational only
// — no assert depends on a measurement.
// You're done when the last line printed is: ALL TESTS PASSED

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
// Each field must sit at an offset divisible by its alignment (double: 8,
// int: 4, char: 1), the compiler may NOT reorder fields for you — it can only
// insert invisible padding bytes — and the total size rounds up to a multiple
// of the largest field alignment.
// JAVA: the JVM reorders fields behind your back (plus a 12-16 byte object
//       header you never see), so declaration order never mattered. Here it does.
struct BadOrder {   // predict this sizeof on paper before checking
    char   ready;
    double timestamp;
    char   flags;
    int    id;
};

// TODO: reorder these SAME four fields (largest alignment first) so that
// sizeof(GoodOrder) shrinks. Right now it is a copy of BadOrder's order.
struct GoodOrder {
    char   ready;
    double timestamp;
    char   flags;
    int    id;
};

// Return {sizeof(BadOrder), sizeof(GoodOrder)}. Predict both numbers on paper
// FIRST — that prediction is the interview skill this drill trains.
std::pair<std::size_t, std::size_t> padded_size_report() {
    // TODO: implement (and fix GoodOrder above — main asserts it got smaller)
    return {0, 0};
}

// ---- Drill 2: traversal order over a row-major matrix -----------------------
// The matrix is ONE flat vector; element (r, c) lives at index r * cols + c —
// row-major: each row's floats sit side by side in memory.
// JAVA: float[][] is an array of POINTERS to separately heap-allocated row
//       objects. A flat float[rows*cols] with manual indexing is the C++ way,
//       and it is how images and tensors actually arrive.
// Sum with r in the OUTER loop, c inner: addresses in increasing order, every
// byte of every cache line used, prefetcher streaming ahead.
double sum_rows_first(const std::vector<float>& m, std::size_t rows,
                      std::size_t cols) {
    // TODO: implement (accumulate into a double)
    return 0.0;
}

// Same elements, same Big-O, loops swapped: c outer, r inner. The inner loop
// hops `cols * 4` bytes per step, using 4 bytes of each cache line it drags in.
double sum_cols_first(const std::vector<float>& m, std::size_t rows,
                      std::size_t cols) {
    // TODO: implement (same indexing m[r * cols + c], loops swapped)
    return 0.0;
}

// ---- Drill 3: contiguous vs pointer-chasing ---------------------------------
// std::vector<int>: the ints themselves, back to back — one cache line holds
// 16-32 of them.
// JAVA: even ArrayList<Integer> pointer-chases — it is an array of references
//       to boxed Integer objects scattered on the heap.
long long sum_vector(const std::vector<int>& values) {
    // TODO: implement (range-for, accumulate into long long)
    return 0;
}

// std::list<int>: a doubly-linked list — every element its own heap allocation
// holding {prev, next, value}. Each ++it is a dependent pointer load.
// The loop body is IDENTICAL to sum_vector's — only the layout differs.
long long sum_list(const std::list<int>& values) {
    // TODO: implement
    return 0;
}

// ---- Drill 4: AoS vs SoA ----------------------------------------------------
// One detected object from a vision model. Six 4-byte fields, alignment 4:
// no padding anywhere — predict sizeof(Detection) yourself.
struct Detection {
    float x, y, w, h;   // box center + size, pixels
    float score;        // confidence in [0, 1]
    int   class_id;
};

// AoS (array of structs): vector<Detection>. Scan d.score for the max; return
// -1.0f for an empty frame (real scores are >= 0). Note what the loop drags
// through the cache: 24 bytes per element to use 4.
float top_score_aos(const std::vector<Detection>& dets) {
    // TODO: implement
    return -1.0f;
}

// SoA (struct of arrays): the scores alone, packed. Same answer (and the same
// -1.0f empty sentinel), 1/6 the memory traffic, SIMD-friendly.
float top_score_soa(const std::vector<float>& scores) {
    // TODO: implement
    return -1.0f;
}

// ---- Drill 5: reserve() and reallocation ------------------------------------
// Build a vector<int> of n elements with push_back and return how many times
// capacity() CHANGED along the way — each change is a reallocation (allocate
// bigger block, copy everything, free old block).
// JAVA: ArrayList grows ~1.5x per overflow; ensureCapacity(n) is reserve(n).
// Here: call reserve(n) first, then push_back n times. Track capacity from
// construction onward (a fresh vector has capacity 0), counting the reserve's
// own jump too. Empirically on this libc++ the answer is exactly 1.
int fill_with_reserve(std::size_t n) {
    // TODO: implement — remember `std::size_t cap = v.capacity();` then
    // compare after reserve() and after every push_back
    return 0;
}

// Same, WITHOUT reserve: watch capacity double 1, 2, 4, 8, ... as it overflows.
int fill_without_reserve(std::size_t n) {
    // TODO: implement
    return 0;
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
