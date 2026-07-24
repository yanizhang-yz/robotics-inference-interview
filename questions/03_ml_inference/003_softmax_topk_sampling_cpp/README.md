# Quest 12: C++ Softmax, Top-K, And Sampling

## Interview Prompt

Implement CPU-side helpers for model output processing: numerically stable softmax,
argmax, and top-k index selection.

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

```bash
cd "/Users/yanizhang/Documents/Inference engineer/robotics-inference-lab"
CPP_QUEST_IMPL=starter .venv/bin/python -m pytest -q quests/12-cpp-softmax-topk-sampling/tests/test_softmax_topk_sampling.py
```
