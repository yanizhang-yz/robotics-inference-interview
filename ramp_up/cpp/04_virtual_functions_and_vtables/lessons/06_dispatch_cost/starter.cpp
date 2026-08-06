// DRILL BRIEF
// Concept: Put a runtime-dispatch boundary around coarse batch work.
// Scenario: Send one span to an inference backend instead of dispatching per item.
// Implement: CountingBackend::infer_batch and run_batch.
// Behavior: Sum the inputs with one batch virtual call and zero per-item calls.
// Example: the asserted batch sum uses exactly one virtual call. Edge: per-item virtual-call count remains zero.
// Interview focus: Reason about dispatch placement without unreliable timing claims.
// Tests: main checks the result and explicit batch/item call counters.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/lessons/06_dispatch_cost -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <iostream>
#include <span>
#include <vector>

struct InferenceBackend {
    virtual ~InferenceBackend() = default;
    virtual long long infer_batch(std::span<const int> inputs) const = 0;
};

class CountingBackend final : public InferenceBackend {
public:
    long long infer_batch(std::span<const int> inputs) const override {
        // TODO: record one coarse call and process the whole span here.
        (void)inputs;
        return 0;
    }

    mutable int batch_calls = 0;
    mutable int item_calls = 0;
};

long long run_batch(
    const InferenceBackend& backend, std::span<const int> inputs
) {
    // TODO: cross the virtual boundary once.
    (void)backend;
    (void)inputs;
    return 0;
}

int main() {
    CountingBackend backend;
    const std::vector<int> inputs{2, 3, 5, 7};
    assert(run_batch(backend, inputs) == 17);
    assert(backend.batch_calls == 1);
    assert(backend.item_calls == 0);
    std::cout << "ALL TESTS PASSED\n";
}
