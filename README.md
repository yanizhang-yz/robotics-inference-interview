# Robotics Inference Interview

Test-driven coding practice for robotics inference roles: algorithms, computer
vision, ML inference, robotics math, linear algebra, and C++ systems.

The migrated C++20 track defines an input/output contract, starter
implementation, reference implementation, automated tests, complexity
analysis, and production-oriented follow-ups for every question. The Python
track retains its original problem statements, starters, solutions, and tests.

## Part of the Software-to-Robotics-Inference Path

For a Java/C-oriented software engineer, first bridge into the interview
languages with the [Python ramp](ramp_up/python/) and [C++ ramp](ramp_up/cpp/).
Then use this repository to turn [M0 — skill translation](https://github.com/yanizhang-yz/software-to-robotics-inference/blob/main/docs/milestones/m0-skill-translation.md)
into practiced implementations and carry completed artifacts into
[M6 — contribute and present the evidence](https://github.com/yanizhang-yz/software-to-robotics-inference/blob/main/docs/milestones/m6-contribute-and-present.md).

## Setup

```bash
cd robotics-inference-interview
uv sync
uv run pytest                        # verify reference solutions (all tests)
uv run pytest questions/01_algorithms # run one category
uv run pytest -v -k "nms"            # run a specific question
```

## How to practice

[Follow the milestone-to-interview practice map](docs/milestone-to-interview-map.md)
to connect completed exercises to coding, system-design, and artifact stories.

1. Pick a question and read its `README.md`.
2. Write your own solution in `starter.py` or `starter.hpp` (don't peek at the
   reference implementation).
3. Run the tests **against your attempt**:

```bash
PRACTICE=1 uv run pytest questions/01_algorithms/001_sliding_window_maximum -v
```

4. Compare with `solution.py` or `reference.hpp` and note the differences.

## Structure

```
questions/               # Interview questions by topic
├── 01_algorithms/       # Data structures, sliding window, heaps, graphs
├── 02_computer_vision/  # NMS, IoU, convolution, image transforms
├── 03_ml_inference/     # Quantization, batching, latency, TensorRT concepts
├── 04_robotics/         # Rotation matrices, kinematics, coordinate frames
├── 05_systems/          # Ownership, memory layout, queues, concurrency
├── 06_linear_algebra/   # Eigenvectors, least squares, PCA
└── 07_llm_and_agents/   # Sampling, KV cache, agentic tool loops

ramp_up/                 # Language fluency drills (for devs coming from Java)
├── python/              # LEARNING_POINTS.md + 8 drill sets — do these FIRST
├── cpp/                 # LEARNING_POINTS.md + 6 drill sets (STL, RAII, moves, vtables, cache, threads)
├── rust/                # LEARNING_POINTS.md (exercises coming)
└── typescript/          # LEARNING_POINTS.md (exercises coming)
```

## Language ramp-up (coming from Java?)

If you think in Java (or another language) and the friction is *writing* Python, start with
[`ramp_up/python/LEARNING_POINTS.md`](ramp_up/python/LEARNING_POINTS.md) — a Java→Python
reference card — then work through the drill sets in order:

| # | Drill set | What it replaces from Java |
|---|-----------|---------------------------|
| 01 | Collections & slicing | Arrays / ArrayList / Collections.sort |
| 02 | Comprehensions & generators | Streams API |
| 03 | Strings & text | String / StringBuilder |
| 04 | Dicts, sets, counters | HashMap / HashSet boilerplate |
| 05 | Functions & closures | Functional interfaces, method refs |
| 06 | Classes & dataclasses | POJOs / Lombok / interfaces |
| 07 | Iteration, heapq, bisect | Iterator / PriorityQueue / TreeMap |
| 08 | NumPy essentials | (no Java equivalent — vectorization) |

Same practice flow as the questions: code in `starter.py`, check with
`PRACTICE=1 uv run pytest ramp_up/python/01_collections_and_slicing -v`.
C++ drills compile and run via the same pytest command (needs `clang++` or `g++`).

The C++ track goes deeper — it's the language of the inference hot path:

| # | Drill set | Interview topic it covers |
|---|-----------|---------------------------|
| 01 | STL containers | vector/string/map fluency, pass-by-const-reference |
| 02 | Ownership & RAII | unique_ptr, destructors as deterministic cleanup |
| 03 | Move semantics & Rule of Five | why copies are silent perf killers, std::move |
| 04 | Virtual functions & vtables | the virtual-destructor classic, dispatch cost |
| 05 | Memory layout & cache | padding, cache lines, AoS/SoA, traversal order |
| 06 | Threads, atomics, queues | data races, mutex vs atomic, producer/consumer |

Each question folder contains:

| Part | Python | Normalized C++20 track |
|------|--------|------------------------|
| Problem statement | `README.md` | `README.md` |
| Explicit contract, complexity, follow-ups | Varies by question | `README.md` |
| Starter implementation | `starter.py` | `starter.hpp` |
| Reference implementation | `solution.py` | `reference.hpp` |
| Automated tests | `test_solution.py` | `test_solution.py` + `test_driver.cpp` |

The normalized C++20 questions also include `DISCUSSION.md` for
production-oriented design analysis. Tests run against the reference by
default and against the starter when `PRACTICE=1` is set.

## Questions

### 01 — Algorithms

| # | Question | Language | Focus |
|---|----------|----------|-------|
| 001 | [Sliding Window Maximum](questions/01_algorithms/001_sliding_window_maximum/) | Python | Monotonic deque |
| 002 | [Median of a Data Stream](questions/01_algorithms/002_median_of_data_stream/) | Python | Two heaps |
| 003 | [K Closest Points](questions/01_algorithms/003_k_closest_points/) | Python | Heap, squared distance |

### 02 — Computer Vision

| # | Question | Language | Focus |
|---|----------|----------|-------|
| 001 | [Non-Maximum Suppression](questions/02_computer_vision/001_non_maximum_suppression/) | Python | IoU, greedy suppression |
| 002 | [Conv2d from Scratch](questions/02_computer_vision/002_conv2d_from_scratch/) | Python | Padding, stride, output shape |
| 003 | [Connected Components](questions/02_computer_vision/003_connected_components/) | Python | Grid BFS, blob labeling |

### 03 — ML Inference

| # | Question | Language | Focus |
|---|----------|----------|-------|
| 001 | [Quantize Tensor (INT8)](questions/03_ml_inference/001_quantize_tensor/) | Python | Symmetric quantization |
| 002 | [Dynamic Batcher](questions/03_ml_inference/002_dynamic_batcher_cpp/) | C++20 | Batching, futures, shutdown |
| 003 | [Softmax, Top-K, and Sampling](questions/03_ml_inference/003_softmax_topk_sampling_cpp/) | C++20 | Stable output processing |
| 004 | [Scaled Dot-Product Attention](questions/03_ml_inference/004_scaled_dot_product_attention/) | Python | Stable softmax, causal mask |

### 04 — Robotics

| # | Question | Language | Focus |
|---|----------|----------|-------|
| 001 | [3D Rotation Matrix](questions/04_robotics/001_rotation_matrix_3d/) | Python | SO(3), coordinate frames |
| 002 | [Quaternion Operations](questions/04_robotics/002_quaternion_operations/) | Python | Hamilton product, to-matrix |
| 003 | [A* on an Occupancy Grid](questions/04_robotics/003_astar_occupancy_grid/) | Python | Heap, admissible heuristic |
| 004 | [Sensor Stream Alignment](questions/04_robotics/004_sensor_stream_alignment_cpp/) | C++20 | Timestamped sensor fusion |
| 005 | [1-D Kalman Filter](questions/04_robotics/005_kalman_filter_1d/) | Python | Predict/update, gain intuition |

### 05 — Systems

| # | Question | Language | Focus |
|---|----------|----------|-------|
| 001 | [Thread-Safe Ring Buffer](questions/05_systems/001_thread_safe_ring_buffer/) | Python | Fixed-capacity concurrency |
| 002 | [Ownership and Memory Layout](questions/05_systems/002_ownership_memory_layout_cpp/) | C++20 | Ownership, views, moves |
| 003 | [Bounded Blocking Queue](questions/05_systems/003_bounded_blocking_queue_cpp/) | C++20 | Backpressure, shutdown |
| 004 | [Ring Buffer](questions/05_systems/004_ring_buffer_cpp/) | C++20 | Allocation-free FIFO storage |
| 005 | [Lock-Free SPSC Queue](questions/05_systems/005_lock_free_spsc_queue_cpp/) | C++20 | Atomics, memory ordering |
| 006 | [Thread Pool Inference Server](questions/05_systems/006_thread_pool_inference_server_cpp/) | C++20 | Workers, tasks, futures |
| 007 | [Memory Pool and Allocation Benchmark](questions/05_systems/007_memory_pool_benchmark_cpp/) | C++20 | Fixed-block allocation |
| 008 | [LRU Cache](questions/05_systems/008_lru_cache/) | Python | Dict + doubly-linked list |

### 06 — Linear Algebra

| # | Question | Language | Focus |
|---|----------|----------|-------|
| 001 | [Power Iteration](questions/06_linear_algebra/001_power_iteration/) | Python | Dominant eigenpairs |
| 002 | [Least-Squares Fit](questions/06_linear_algebra/002_least_squares_fit/) | Python | Normal equations, lstsq |

### 07 — LLM & Agents

| # | Question | Language | Focus |
|---|----------|----------|-------|
| 001 | [Sampling: Temperature, Top-k, Top-p](questions/07_llm_and_agents/001_sampling_topk_topp/) | Python | Serving-config math |
| 002 | [KV Cache](questions/07_llm_and_agents/002_kv_cache/) | Python | Memory formula, O(n) decode |
| 003 | [Agent Tool Loop](questions/07_llm_and_agents/003_agent_tool_loop/) | Python | Transcript state, error recovery |

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for the question contract and validation
commands.

---

*Built by Yani Zhang — robotics inference interview prep, open for anyone.*
