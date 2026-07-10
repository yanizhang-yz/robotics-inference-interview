# Robotics Inference Engineer — Interview Prep

Coding questions, solutions, and test suites for robotics inference engineer interviews. Built for hands-on practice: read the problem, write your own solution, then run the tests.

## Setup

```bash
cd robotics-inference-interview
uv sync
uv run pytest                        # run all tests
uv run pytest questions/01_algorithms # run one category
uv run pytest -v -k "nms"            # run a specific question
```

## Structure

```
questions/
├── 01_algorithms/       # Data structures, sliding window, heaps, graphs
├── 02_computer_vision/  # NMS, IoU, convolution, image transforms
├── 03_ml_inference/     # Quantization, batching, latency, TensorRT concepts
├── 04_robotics/         # Rotation matrices, kinematics, coordinate frames
├── 05_systems/          # Memory layout, CUDA basics, concurrency
└── 06_linear_algebra/   # SVD, PCA, eigendecomposition
```

Each question folder contains:
| File | Purpose |
|------|---------|
| `README.md` | Problem statement, context, follow-up questions |
| `solution.py` | Reference solution |
| `test_solution.py` | pytest test suite |

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
