# Quest 12: C++ Softmax, Top-K, And Sampling

## Interview Prompt

Implement CPU-side helpers for model output processing: numerically stable softmax,
argmax, and top-k index selection.

## Input and Output Contract

The public interface is the declaration shown below. Inputs, return values,
blocking behavior, ownership rules, and shutdown behavior are part of the
contract and are exercised by `test_solution.py` plus `test_driver.cpp`.

```cpp
std::vector<float> softmax(std::span<const float> logits);
std::size_t argmax(std::span<const float> values);
std::vector<std::size_t> top_k_indices(std::span<const float> values,
                                       std::size_t k);
```

## Requirements

Implement:

```cpp
std::vector<float> softmax(std::span<const float> logits);
std::size_t argmax(std::span<const float> values);
std::vector<std::size_t> top_k_indices(std::span<const float> values,
                                       std::size_t k);
```

Behavior:

- `softmax` is numerically stable for large logits
- softmax probabilities sum to approximately `1.0`
- `argmax` returns the first index with the largest value
- `top_k_indices` returns indices sorted by descending value
- invalid empty input or `k == 0` throws

## System Context

This sits after model logits:

```text
model forward
-> logits
-> softmax / top-k / sampling
-> selected token or action
```

For robotics, this can map to discrete action heads, grasp candidates, or language
token generation in a VLA-style policy.

## Why It Exists

Inference is not only model execution. Serving systems also do output processing,
sampling, filtering, and validation before returning a result.

## Problem Solved

This quest trains numerics, vector operations, cache-friendly loops, and model-output
vocabulary.

## Failure Modes

- naive exponentials overflow
- probabilities do not sum to one
- top-k returns unstable or unordered indices
- empty inputs crash or produce undefined behavior

## Metrics

- latency per logits vector
- allocations per call
- numerical error
- throughput over many vectors

## Improvements After The Basic Version

- in-place softmax
- temperature
- top-p sampling
- SIMD
- GPU/Triton version later

## Interview Follow-Ups

- Why subtract the max before exponentiating?
- What is the difference between argmax and sampling?
- Why might top-k allocate too much?
- How would you benchmark this helper?

## Player Task

Run the starter implementation:

```bash
PRACTICE=1 uv run pytest questions/03_ml_inference/003_softmax_topk_sampling_cpp -q
```

Run the reference implementation:

```bash
uv run pytest questions/03_ml_inference/003_softmax_topk_sampling_cpp -q
```

## Complexity Targets

For `n` input values, `softmax` and `argmax` run in `O(n)` time; `softmax` returns
`O(n)` storage while `argmax` uses `O(1)` auxiliary space. `top_k_indices` runs in
`O(n log k)` time and uses `O(n)` storage because it builds an index vector for all
inputs before the partial sort. There is no synchronization. The bounds treat each
floating-point operation, comparison, and exponential as constant time and exclude
allocator latency for returned and working vectors, while still counting their
storage.

## Interview Follow-ups

1. Which invariant makes this implementation correct?
2. What fails first under overload or malformed input?
3. Which metric would reveal that failure in production?
4. What changes for a robot control loop versus an offline batch job?
