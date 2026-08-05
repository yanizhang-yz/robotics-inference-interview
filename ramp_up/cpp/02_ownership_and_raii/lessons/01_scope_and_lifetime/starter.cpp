// DRILL BRIEF
// Concept: Local objects live until their scope ends, then die in reverse construction order.
// Scenario: Trace nested sensor-session scopes named outer, inner-a, and inner-b.
// Implement: scope_trace using the provided ScopeMarker and nested braces.
// Behavior: Record all construction/destruction events, with after-inner before outer dies.
// Interview focus: Predict the exact event sequence from construction order and braces.
// Tests: main compares the complete seven-event trace.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/01_scope_and_lifetime -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

class ScopeMarker {
public:
    ScopeMarker(std::string name, std::vector<std::string>& events)
        : name_(std::move(name)), events_(events) {
        events_.push_back("construct " + name_);
    }
    ~ScopeMarker() { events_.push_back("destroy " + name_); }

private:
    std::string name_;
    std::vector<std::string>& events_;
};

std::vector<std::string> scope_trace() {
    // Exercise: create the three markers in the required nested scopes and
    // record "after-inner" after the inner scope has ended.
    return {};
}

int main() {
    const std::vector<std::string> expected{
        "construct outer", "construct inner-a", "construct inner-b",
        "destroy inner-b", "destroy inner-a", "after-inner", "destroy outer"};
    assert(scope_trace() == expected);
    std::cout << "ALL TESTS PASSED\n";
}
