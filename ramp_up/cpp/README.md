# C++ for Robotics Inference

This self-paced track is for experienced programmers who are new to C++.
It teaches the language mechanisms that control ownership, lifetime, memory
layout, concurrency, and performance in robotics and inference systems.

## Choose a route

- Play it: run `./drill` from the repo root — module 01 as an interactive
  card → predict → code loop with instant compile feedback and a staged
  capstone. Start here if you learn by doing.
- Concept tour: read [CONCEPT_MAP.md](CONCEPT_MAP.md) before implementing.
- Guided path: complete micro-lessons in order, then the module capstone.
- Fast path: attempt a module capstone directly and return to any lesson it
  exposes.

## Practice contract

Reference:
`uv run pytest ramp_up/cpp/<module-or-lesson> -v`

Learner attempt:
`PRACTICE=1 uv run pytest ramp_up/cpp/<module-or-lesson> -v`

## Modules

1. [Core language and STL](01_stl_containers/)
2. [Ownership and RAII](02_ownership_and_raii/)
3. [Move semantics and resource-aware values](03_move_semantics_rule_of_five/)
4. [Runtime polymorphism](04_virtual_functions_and_vtables/)
5. [Memory layout and cache behavior](05_memory_layout_and_cache/)
6. [Threads, atomics, and queues](06_threads_atomics_queues/)

Keep [LEARNING_POINTS.md](LEARNING_POINTS.md) nearby as a compact reference.

## Completion

A concept is complete when you can predict its behavior, explain why it
exists, implement the exercise, pass the test, and connect it to the stated
robotics or inference application.
