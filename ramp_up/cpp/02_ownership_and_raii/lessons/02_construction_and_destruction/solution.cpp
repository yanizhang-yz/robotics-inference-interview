#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

struct TracePart {
    TracePart(std::string part_name, std::vector<std::string>& trace)
        : name(std::move(part_name)), events(trace) {
        events.push_back("construct " + name);
    }
    ~TracePart() { events.push_back("destroy " + name); }
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
