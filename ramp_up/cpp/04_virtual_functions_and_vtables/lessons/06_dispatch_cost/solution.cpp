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
        ++batch_calls;
        long long total = 0;
        for (int input : inputs) total += input;
        return total;
    }

    mutable int batch_calls = 0;
    mutable int item_calls = 0;
};

long long run_batch(
    const InferenceBackend& backend, std::span<const int> inputs
) {
    return backend.infer_batch(inputs);
}

int main() {
    CountingBackend backend;
    const std::vector<int> inputs{2, 3, 5, 7};
    assert(run_batch(backend, inputs) == 17);
    assert(backend.batch_calls == 1);
    assert(backend.item_calls == 0);
    std::cout << "ALL TESTS PASSED\n";
}
