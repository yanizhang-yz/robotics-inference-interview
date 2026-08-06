// DRILL BRIEF
// Concept: Integrate layout, traversal, container, AoS/SoA, and growth reasoning.
// Scenario: Review the memory behavior of an inference data path end to end.
// Implement: The existing five drill groups in this file.
// Behavior: Preserve exact results while making each memory decision explicit.
// Example: row-first and column-first sums match for the asserted matrices. Edge: empty score inputs return -1 and timing output never controls correctness.
// Interview focus: Derive structural behavior; timing lines are informational.
// Tests: main retains its existing result and invariant assertions unchanged.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache/test_solution.py -q
// Done when: The last line printed is ALL TESTS PASSED.

// 05_memory_layout_and_cache — YOUR attempt. Fill in the TODO bodies (and fix
// one struct's field order), then run:
//   PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache/test_solution.py -v
// Or compile and run directly:
//   clang++ -std=c++20 -Wall -Wextra -Werror=return-type -o /tmp/mem starter.cpp && /tmp/mem
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

// Provided stopwatch; timing output is informational and never graded.
static double ms_since(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration<double, std::milli>(
               std::chrono::steady_clock::now() - start)
        .count();
}

// Drill 1: retain the same fields while producing a denser declaration order.
struct BadOrder {
    char   ready;
    double timestamp;
    char   flags;
    int    id;
};

// TODO: reorder only these four fields so GoodOrder is smaller than BadOrder.
struct GoodOrder {
    char   ready;
    double timestamp;
    char   flags;
    int    id;
};

// Return both object sizes in BadOrder, GoodOrder order.
std::pair<std::size_t, std::size_t> padded_size_report() {
    // TODO: implement after fixing GoodOrder.
    return {0, 0};
}

// Drill 2: sum every row-major matrix element in row-first order.
double sum_rows_first(const std::vector<float>& m, std::size_t rows,
                      std::size_t cols) {
    // TODO: implement.
    return 0.0;
}

// Sum the same elements in column-first order; results must be identical.
double sum_cols_first(const std::vector<float>& m, std::size_t rows,
                      std::size_t cols) {
    // TODO: implement.
    return 0.0;
}

// Drill 3: sum all values in the contiguous and node-based inputs.
long long sum_vector(const std::vector<int>& values) {
    // TODO: implement.
    return 0;
}

// The list result must match the vector result for equal values.
long long sum_list(const std::list<int>& values) {
    // TODO: implement
    return 0;
}

// Drill 4: compare equivalent detection-score queries over AoS and SoA layouts.
struct Detection {
    float x, y, w, h;
    float score;
    int   class_id;
};

// Return the highest AoS score, or -1.0f for empty input.
float top_score_aos(const std::vector<Detection>& dets) {
    // TODO: implement
    return -1.0f;
}

// Return the highest SoA score with the same empty-input contract.
float top_score_soa(const std::vector<float>& scores) {
    // TODO: implement
    return -1.0f;
}

// Drill 5: build n values and report capacity changes with an up-front reserve.
int fill_with_reserve(std::size_t n) {
    // TODO: implement the specified count.
    return 0;
}

// Build the same n values without reserving and report the observed count.
int fill_without_reserve(std::size_t n) {
    // TODO: implement
    return 0;
}

int main() {
    std::cout << std::fixed << std::setprecision(1);

    // Layout assertions.
    {
        std::pair<std::size_t, std::size_t> report = padded_size_report();
        assert(report.first == sizeof(BadOrder));
        assert(report.second == sizeof(GoodOrder));
        assert(report.first >= 14 && report.second >= 14);
        assert(report.second < report.first);
        std::cout << "[layout] BadOrder = " << report.first
                  << " bytes, GoodOrder = " << report.second
                  << " bytes (same four fields)\n";
        assert(sizeof(Detection) == 6 * 4);
    }

    // Traversal assertions.
    {
        // Small exact case.
        std::vector<float> small(12);
        for (std::size_t i = 0; i < 12; ++i) {
            small[i] = static_cast<float>(i + 1);
        }
        assert(sum_rows_first(small, 3, 4) == 78.0);
        assert(sum_cols_first(small, 3, 4) == 78.0);

        const std::size_t R = 2048, C = 2048;
        std::vector<float> m(R * C);
        for (std::size_t i = 0; i < m.size(); ++i) {
            m[i] = static_cast<float>(i % 7);
        }
        auto t0 = std::chrono::steady_clock::now();
        double rows = sum_rows_first(m, R, C);
        double rows_ms = ms_since(t0);
        t0 = std::chrono::steady_clock::now();
        double cols = sum_cols_first(m, R, C);
        double cols_ms = ms_since(t0);
        assert(rows == cols);
        std::cout << "[timing] 2048x2048 sum: rows-first " << rows_ms
                  << " ms, cols-first " << cols_ms << " ms\n";
    }

    // Container assertions.
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
        assert(sv > 0);
        std::cout << "[timing] 2M ints: vector " << vec_ms << " ms, list "
                  << list_ms << " ms\n";
    }

    // Layout-equivalence assertions.
    {
        std::vector<Detection> tiny = {
            {10.f, 20.f, 5.f, 5.f, 0.30f, 3},
            {40.f, 25.f, 9.f, 7.f, 0.90f, 0},
            {12.f, 80.f, 4.f, 6.f, 0.75f, 3},
        };
        assert(top_score_aos(tiny) == 0.90f);
        assert(top_score_soa({0.30f, 0.90f, 0.75f}) == 0.90f);
        assert(top_score_aos({}) == -1.0f);
        assert(top_score_soa({}) == -1.0f);

        const std::size_t N = 2'000'000;
        std::vector<Detection> aos(N);
        std::vector<float> scores(N);
        for (std::size_t i = 0; i < N; ++i) {
            float s = static_cast<float>((i * 2654435761u) % 1000) / 1000.0f;
            aos[i] = {1.f, 2.f, 3.f, 4.f, s, static_cast<int>(i % 80)};
            scores[i] = s;
        }
        aos[N / 2].score = 1.5f;
        scores[N / 2] = 1.5f;
        auto t0 = std::chrono::steady_clock::now();
        float best_aos = top_score_aos(aos);
        double aos_ms = ms_since(t0);
        t0 = std::chrono::steady_clock::now();
        float best_soa = top_score_soa(scores);
        double soa_ms = ms_since(t0);
        assert(best_aos == 1.5f);
        assert(best_soa == 1.5f);
        assert(best_aos == best_soa);
        std::cout << "[timing] 2M detections, top score: AoS " << aos_ms
                  << " ms, SoA " << soa_ms << " ms\n";
    }

    // Capacity-growth assertions.
    {
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
