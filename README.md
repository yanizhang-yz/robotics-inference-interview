# Robotics Inference Engineer — Interview Prep

Coding questions, solutions, and test suites for robotics inference engineer interviews. Built for hands-on practice: read the problem, write your own solution, then run the tests.

## Setup

```bash
cd robotics-inference-interview
uv sync
uv run pytest                        # verify reference solutions (all tests)
uv run pytest questions/01_algorithms # run one category
uv run pytest -v -k "nms"            # run a specific question
```

## How to practice

1. Pick a question and read its `README.md`.
2. Write your own solution in `starter.py` (don't peek at `solution.py`).
3. Run the tests **against your attempt**:

```bash
PRACTICE=1 uv run pytest questions/01_algorithms/001_sliding_window_maximum -v
```

4. Compare with the reference in `solution.py` and note the differences.

## Structure

```
questions/               # Interview questions by topic
├── 01_algorithms/       # Data structures, sliding window, heaps, graphs
├── 02_computer_vision/  # NMS, IoU, convolution, image transforms
├── 03_ml_inference/     # Quantization, batching, latency, TensorRT concepts
├── 04_robotics/         # Rotation matrices, kinematics, coordinate frames
├── 05_systems/          # Memory layout, CUDA basics, concurrency
└── 06_linear_algebra/   # SVD, PCA, eigendecomposition

ramp_up/                 # Language fluency drills (for devs coming from Java)
├── python/              # LEARNING_POINTS.md + 8 drill sets — do these FIRST
├── cpp/                 # LEARNING_POINTS.md + STL & RAII drills
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

Each question folder contains:
| File | Purpose |
|------|---------|
| `README.md` | Problem statement, context, follow-up questions |
| `starter.py` | Skeleton with signatures + hints — write your attempt here |
| `solution.py` | Reference solution |
| `test_solution.py` | pytest test suite (runs against reference, or your starter with `PRACTICE=1`) |

C++ questions (when added) follow the same pattern: `starter.cpp`, `solution.cpp`, and a test harness.

## Questions

### 01 — Algorithms
| # | Question | Difficulty | Key concept |
|---|----------|-----------|-------------|
| 001 | [Sliding Window Maximum](questions/01_algorithms/001_sliding_window_maximum/) | Medium | Monotonic deque |

### 02 — Computer Vision
| # | Question | Difficulty | Key concept |
|---|----------|-----------|-------------|
| 001 | [Non-Maximum Suppression](questions/02_computer_vision/001_non_maximum_suppression/) | Medium | IoU, greedy |

### 03 — ML Inference
| # | Question | Difficulty | Key concept |
|---|----------|-----------|-------------|
| 001 | [Quantize Tensor (INT8)](questions/03_ml_inference/001_quantize_tensor/) | Medium | Symmetric quant, scale |

### 04 — Robotics
| # | Question | Difficulty | Key concept |
|---|----------|-----------|-------------|
| 001 | [3D Rotation Matrix](questions/04_robotics/001_rotation_matrix_3d/) | Medium | SO(3), coordinate frames |

### 05 — Systems
*(coming soon)*

### 06 — Linear Algebra
*(coming soon)*

## Contributing

Open a PR or issue with a new question. Each PR should add a full folder: `README.md` + `solution.py` + `test_solution.py`.

---

*Built by Yani Zhang — robotics inference interview prep, open for anyone.*
