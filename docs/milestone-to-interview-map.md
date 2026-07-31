# Milestone-to-Interview Practice Map

This map is for the Java/C-oriented software-engineer persona in the
[Software-to-Robotics-Inference guide](https://github.com/yanizhang-yz/software-to-robotics-inference).
Complete an exercise before using it as an artifact story; a starter file alone
is not evidence.

## [M0 — Translate Your Existing Software Skills](https://github.com/yanizhang-yz/software-to-robotics-inference/blob/main/docs/milestones/m0-skill-translation.md)

**Practice directories:** [Python ramp](../ramp_up/python/) and [C++ ramp](../ramp_up/cpp/).

- **Coding prompt:** Implement a fixed-capacity queue in the language that is
  less familiar to you, then explain the Java/C construct it replaces.
- **System-design follow-up:** Describe how you would split an existing Java/C
  service into Python orchestration and a C++ inference hot path while keeping
  ownership and failure boundaries explicit.
- **Completed-artifact story prompt:** Choose a completed Python or C++ ramp
  drill. What habit from Java/C did you have to change, how did the tests expose
  the gap, and what would you now do differently in production code?

## [M1 — Execute and Measure a Model](https://github.com/yanizhang-yz/software-to-robotics-inference/blob/main/docs/milestones/m1-model-inference.md)

**Practice directories:** [Quantize Tensor (INT8)](../questions/03_ml_inference/001_quantize_tensor/)
and [Dynamic Batcher](../questions/03_ml_inference/002_dynamic_batcher_cpp/).

- **Coding prompt:** Implement symmetric INT8 quantization and account for the
  scale, clipping, and reconstruction error.
- **System-design follow-up:** Given a p99 latency budget, explain how you
  would measure the accuracy-throughput-latency trade-off of quantization and
  bounded dynamic batching.
- **Completed-artifact story prompt:** Walk through a completed quantization or
  batching artifact: its contract, the command that verified it, the latency or
  correctness observation you recorded, and the limitation you would disclose.

## [M4 — Build the Robotics Inference Loop](https://github.com/yanizhang-yz/software-to-robotics-inference/blob/main/docs/milestones/m4-robotics-inference-loop.md)

**Practice directories:** [Bounded Blocking Queue](../questions/05_systems/003_bounded_blocking_queue_cpp/),
[Ring Buffer](../questions/05_systems/004_ring_buffer_cpp/),
[Sensor Stream Alignment](../questions/04_robotics/004_sensor_stream_alignment_cpp/),
[Thread Pool Inference Server](../questions/05_systems/006_thread_pool_inference_server_cpp/),
and [Memory Pool and Allocation Benchmark](../questions/05_systems/007_memory_pool_benchmark_cpp/).

- **Coding prompt:** Implement the bounded queue used between sensor ingestion
  and inference, including shutdown behavior and backpressure.
- **System-design follow-up:** Design a sensor-to-action loop that chooses
  timestamp alignment, ring-buffer capacity, thread-pool ownership, and
  memory-pool lifetime under a fixed control deadline.
- **Completed-artifact story prompt:** Use a completed queue, alignment,
  thread-pool, or memory-pool artifact to explain the invariant you tested,
  the concurrency or allocation trade-off you made, and the unmeasured
  real-world risk that remains.

## [M5 — Serve a Modern Language or VLA Model with SGLang](https://github.com/yanizhang-yz/software-to-robotics-inference/blob/main/docs/milestones/m5-sglang-serving.md)

**Practice directories:** [Dynamic Batcher](../questions/03_ml_inference/002_dynamic_batcher_cpp/)
and [Softmax, Top-K, and Sampling](../questions/03_ml_inference/003_softmax_topk_sampling_cpp/).

- **Coding prompt:** Implement numerically stable softmax followed by top-k
  filtering and sampling, including the edge cases in the contract.
- **System-design follow-up:** Explain how a serving layer should combine
  dynamic batching with decoding controls without letting queue delay violate a
  request deadline.
- **Completed-artifact story prompt:** Present a completed batching or sampling
  artifact: why its API contract matters to callers, how you verified the
  output behavior, and which serving-scale measurement is still missing.

## [M6 — Contribute and Present the Evidence](https://github.com/yanizhang-yz/software-to-robotics-inference/blob/main/docs/milestones/m6-contribute-and-present.md)

**Practice directories:** [Dynamic Batcher](../questions/03_ml_inference/002_dynamic_batcher_cpp/),
[Sensor Stream Alignment](../questions/04_robotics/004_sensor_stream_alignment_cpp/),
[Thread Pool Inference Server](../questions/05_systems/006_thread_pool_inference_server_cpp/),
and [Softmax, Top-K, and Sampling](../questions/03_ml_inference/003_softmax_topk_sampling_cpp/).

- **Coding prompt:** Take one completed exercise and add a narrowly scoped
  edge-case test that documents a production-relevant contract boundary.
- **System-design follow-up:** Starting from one completed artifact, outline a
  production design review: interface, metrics, failure modes, rollback, and
  the evidence required before claiming deployment readiness.
- **Completed-artifact story prompt:** Tell the artifact-backed interview
  story: problem, implementation and test command, measured result, reviewer
  feedback or next validation, and the claim you deliberately will not make.
