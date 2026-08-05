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
    std::vector<std::string> events;
    {
        ScopeMarker outer("outer", events);
        {
            ScopeMarker inner_a("inner-a", events);
            ScopeMarker inner_b("inner-b", events);
        }
        events.push_back("after-inner");
    }
    return events;
}

int main() {
    const std::vector<std::string> expected{
        "construct outer", "construct inner-a", "construct inner-b",
        "destroy inner-b", "destroy inner-a", "after-inner", "destroy outer"};
    assert(scope_trace() == expected);
    std::cout << "ALL TESTS PASSED\n";
}
