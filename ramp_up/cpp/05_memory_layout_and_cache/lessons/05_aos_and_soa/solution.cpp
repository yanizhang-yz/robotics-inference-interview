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
    if (detections.empty()) return std::nullopt;
    return std::max_element(
        detections.begin(), detections.end(),
        [](const Detection& a, const Detection& b) {
            return a.score < b.score;
        }
    )->id;
}

std::optional<int> best_detection_soa(const DetectionColumns& detections) {
    assert(detections.ids.size() == detections.scores.size());
    if (detections.scores.empty()) return std::nullopt;
    const auto best = std::max_element(
        detections.scores.begin(), detections.scores.end()
    );
    return detections.ids[
        static_cast<std::size_t>(best - detections.scores.begin())
    ];
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

    // This is a comparison model for the data exposed to each scan. It does
    // not claim that the processor literally transfers every modeled byte.
    const std::size_t aos_scanned_bytes = detections.size() * sizeof(Detection);
    const std::size_t soa_scanned_bytes = columns.scores.size() * sizeof(float);
    assert(aos_scanned_bytes == 3 * sizeof(Detection));
    assert(soa_scanned_bytes == 3 * sizeof(float));
    assert(soa_scanned_bytes < aos_scanned_bytes);
    std::cout << "ALL TESTS PASSED\n";
}
