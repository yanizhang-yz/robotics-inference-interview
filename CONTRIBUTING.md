# Contributing

Contributions should add one complete, test-driven interview question at a
time. Put it in one directory under the matching `questions/<topic>/`
category.

Every question directory must include:

- a `README.md` with the input/output contract, complexity analysis, and
  production-oriented follow-up questions;
- a starter implementation with the public interface and no completed answer;
- a reference implementation;
- automated tests that exercise the contract and select the reference by
  default;
- any test driver needed to compile or run the implementation.

Use `starter.py` and `solution.py` for Python questions. Use `starter.hpp`,
`reference.hpp`, and `test_driver.cpp` for C++ questions. In both cases, name
the pytest entry point `test_solution.py`.

Set up the repository and verify all reference implementations:

```bash
uv sync
uv run pytest -q
```

Before opening a pull request, run the new question against its starter with
`PRACTICE=1`:

```bash
PRACTICE=1 uv run pytest questions/<topic>/<question> -v
```

Practice mode must select the starter implementation so a candidate can work
from the contract and receive useful test feedback without editing the
reference.
