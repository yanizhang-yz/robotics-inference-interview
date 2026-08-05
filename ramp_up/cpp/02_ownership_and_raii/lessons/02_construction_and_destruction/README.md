# 02 — Construction and Destruction

This lesson extends scope lifetime to class members by tracing the capture and
inference parts of a camera pipeline.

## The problem this lesson solves

An object's members can depend on one another. Assuming initializer-list text
controls their order can initialize a dependency too late or destroy it too
early, even when the constructor looks plausible.

## The lesson

A complete object's lifetime includes all member lifetimes. Members always
construct in the order they are **declared in the class**, not the order written
in the constructor's initializer list. They destruct in the reverse declaration
order after the containing object's destructor body finishes.

`PipelineTrace` deliberately lists `inference_` before `capture_` in its
initializer text, while declaring `capture_` first. The observable trace remains
capture then inference on construction, inference then capture on destruction.
Put dependencies above their dependents in the member declarations.

## How interviewers test this

Expect a reordered initializer list and a request to predict actual output, or
to diagnose a member that reads another member before that dependency exists.

## Muscle memory

```cpp
class Pipeline {
    Capture capture_;     // constructs first, destroys last
    Inference inference_; // constructs second, destroys first
};
```

Read member declarations, not initializer punctuation, to determine lifetime.

## The drills

Complete `TracePart` so its constructor records `construct <name>` and its
destructor records `destroy <name>`. Do not reorder `PipelineTrace`: its
misleading initializer text is the experiment.

## How to practice

Edit `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/02_construction_and_destruction -q
```

Drop `PRACTICE=1` to check the reference solution.
