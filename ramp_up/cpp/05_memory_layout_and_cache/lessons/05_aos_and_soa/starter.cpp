// DRILL BRIEF
// Concept: Choose array-of-structs or struct-of-arrays for the fields scanned.
// Scenario: Find the highest-scoring detection ID in an inference result.
// Implement: best_detection_aos and best_detection_soa.
// Behavior: Return the same best ID, or an empty optional for an empty frame.
// Interview focus: Compare field access, SIMD shape, and modeled scanned bytes.
// Tests: main checks IDs, empty results, equal columns, and the byte model.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache/lessons/05_aos_and_soa -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <vector>

struct Detection {
    float x;
    float y;
    float width;
    float height;
    float score;
    int id;
};

struct DetectionColumns {
    std::vector<int> ids;
    std::vector<float> scores;
};

std::optional<int> best_detection_aos(
    const std::vector<Detection>& detections
) {
    // TODO: find the record with the greatest score.
    (void)detections;
    return std::nullopt;
}

std::optional<int> best_detection_soa(const DetectionColumns& detections) {
    assert(detections.ids.size() == detections.scores.size());
    // TODO: find the greatest score and use its index in the ID column.
    return std::nullopt;
}

int main() {
    const std::vector<Detection> detections{
        {1.f, 2.f, 3.f, 4.f, 0.25f, 4},
        {5.f, 6.f, 7.f, 8.f, 0.90f, 8},
        {9.f, 10.f, 11.f, 12.f, 0.60f, 15},
    };
    const DetectionColumns columns{{4, 8, 15}, {0.25f, 0.90f, 0.60f}};
    assert(best_detection_aos(detections) == std::optional<int>{8});
    assert(best_detection_soa(columns) == std::optional<int>{8});
    assert(best_detection_aos(detections) == best_detection_soa(columns));
    assert(!best_detection_aos({}).has_value());
    assert(!best_detection_soa({{}, {}}).has_value());

    // These byte counts model the data exposed to each scan. They do not claim
    // the processor literally transfers every byte counted here.
    const std::size_t aos_scanned_bytes = detections.size() * sizeof(Detection);
    const std::size_t soa_scanned_bytes = columns.scores.size() * sizeof(float);
    assert(aos_scanned_bytes == 3 * sizeof(Detection));
    assert(soa_scanned_bytes == 3 * sizeof(float));
    assert(soa_scanned_bytes < aos_scanned_bytes);
    std::cout << "ALL TESTS PASSED\n";
}
