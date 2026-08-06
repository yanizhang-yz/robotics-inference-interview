// DRILL BRIEF
// Concept: Members construct in declaration order and destruct in reverse order.
// Scenario: Trace capture and inference parts of a camera pipeline.
// Implement: TracePart's constructor/destructor event recording.
// Behavior: capture constructs before inference, then inference dies before capture.
// Example: the trace begins construct capture, construct inference. Edge: reversed initializer text does not change member order.
// Interview focus: Explain why initializer-list text cannot reorder member lifetimes.
// Tests: main checks the trace during and after a PipelineTrace scope.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/02_construction_and_destruction -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

struct TracePart {
    TracePart(std::string part_name, std::vector<std::string>& trace)
        : name(std::move(part_name)), events(trace) {
        // Exercise: record this part's construction.
    }
    ~TracePart() {
        // Exercise: record this part's destruction.
    }
    std::string name;
    std::vector<std::string>& events;
};

class PipelineTrace {
public:
    explicit PipelineTrace(std::vector<std::string>& events)
        : inference_("inference", events), capture_("capture", events) {}

private:
    TracePart capture_;
    TracePart inference_;
};

int main() {
    std::vector<std::string> events;
    {
        PipelineTrace pipeline(events);
        assert((events == std::vector<std::string>{
            "construct capture", "construct inference"}));
    }
    assert((events == std::vector<std::string>{
        "construct capture", "construct inference",
        "destroy inference", "destroy capture"}));
    std::cout << "ALL TESTS PASSED\n";
}
