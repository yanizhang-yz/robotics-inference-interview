# Agent Tool Loop

**Difficulty:** Medium  
**Source:** Increasingly common in agentic-systems interviews; no single-company attribution  
**Tags:** `agents`, `tool-use`, `control-loop`, `error-recovery`, `pure-python`

## Problem

Build the executor at the heart of every agent framework — pure Python, no LLM
calls. The "model" is just a function you're handed: it looks at the transcript
so far and decides the next action.

- `tools` — a dict mapping tool name → plain callable.
- `model(transcript)` — takes the transcript (a list of dicts) and returns either
  `{"type": "tool_call", "name": ..., "arguments": {...}}` or
  `{"type": "final", "answer": ...}`.
- `run_agent(model, tools, task, max_steps=10) -> (answer, transcript)`:
  1. The transcript starts as `[{"type": "task", "content": task}]`.
  2. Each step: call `model(transcript)`, append its output.
  3. On `"final"`: return `(answer, transcript)`.
  4. On `"tool_call"`: validate, run `tools[name](**arguments)`, and append
     `{"type": "tool_result", "name": ..., "result": ...}` — or, for an unknown
     tool / bad call shape / a tool that **raises**, append
     `{"type": "tool_error", "name": ..., "error": <message>}` instead. The loop
     must **never crash**: capture `str(e)` and feed it back so the model can
     read the error and recover.
  5. After `max_steps` model calls without a final answer: `(None, transcript)`.
- `validate_tool_call(call, tools) -> list[str]` — error strings for: missing
  `"name"`, unknown tool, missing `"arguments"`, `"arguments"` not a dict.
  Empty list means the call is well-formed.

```python
tools = {"calculator": lambda a, b: a + b}
answer, transcript = run_agent(model, tools, "add 2+3, then add 10")
# transcript: task -> tool_call -> tool_result -> tool_call -> tool_result -> final
```

## Why this appears in robotics inference interviews

Strip any agent framework to its core and this loop is what's left, so
interviewers use it to test whether you understand the model rather than the
brand names: the **transcript is the state** (the model is stateless — everything
it knows arrives as that list of dicts), **tool results are feedback** (each
result is appended and shapes the next decision), **errors are data** (a raised
exception must come back as a `tool_error` entry the model can read and recover
from — a crashing loop is an instant fail), and the **step budget is the runaway
guard** (an LLM in a loop with no cap is an infinite-loop bug with an API bill).
The robotics tie is exact: a robot task planner invoking perception and
manipulation skills — `detect_objects`, `grasp`, `place`, retry on a failed
grasp — is this loop with different tool names.

## Approach

One `for _ in range(max_steps)` loop — the range *is* the budget:

1. `action = model(transcript)`; append `action` as-is.
2. `"final"` → return `(action["answer"], transcript)`.
3. Otherwise treat it as a tool call: `errors = validate_tool_call(action, tools)`;
   if any, append a `tool_error` with the joined messages and `continue` — the
   model sees why its call was rejected.
4. Valid call: `tools[name](**arguments)` inside `try/except Exception` — success
   appends `tool_result`, an exception appends `tool_error` with `str(e)`.
5. Fall out of the loop → `(None, transcript)`.

`validate_tool_call` checks the dict shape only — it never calls the tool.

**Time:** O(max_steps) model/tool invocations. **Space:** O(transcript) — it
grows by at most two entries per step.

## Follow-ups to be ready for

1. **Parallel tool calls**: the model returns a *list* of calls; run them
   concurrently, append all results before the next model call — and results
   need an id to pair them with their calls once order stops being enough.
2. **Streaming**: surface partial model output and tool progress instead of
   blocking per step.
3. **Retries with backoff vs surfacing to the model**: transient failures
   (timeouts) merit an automatic retry inside the executor; semantic failures
   (bad arguments) should go back to the model, which is the thing that can
   change them.
4. **Context-window trimming**: transcripts grow without bound — drop or
   summarize old tool results while keeping the task and recent steps.
5. **Guardrails**: tool allowlists per task, argument schema validation beyond
   "is a dict", spend/step budgets — `validate_tool_call` is the seed of all of
   these.
