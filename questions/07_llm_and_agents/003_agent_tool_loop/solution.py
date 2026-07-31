"""Minimal agent executor: transcript-as-state, tool-result feedback,
error recovery, step budget.

The model is a plain function from transcript to action, so the loop is
testable without any LLM. Errors — malformed calls, unknown tools, tools
that raise — are appended to the transcript as data for the model to read
and recover from; the loop itself never crashes.
"""


def validate_tool_call(call: dict, tools: dict) -> list[str]:
    """Shape-check a tool-call dict; empty list means well-formed."""
    errors = []
    if "name" not in call:
        errors.append("missing 'name'")
    elif call["name"] not in tools:
        errors.append(f"unknown tool: {call['name']!r}")
    if "arguments" not in call:
        errors.append("missing 'arguments'")
    elif not isinstance(call["arguments"], dict):
        errors.append("'arguments' must be a dict")
    return errors


def run_agent(model, tools: dict, task: str, max_steps: int = 10):
    """Run the loop; return (answer, transcript), or (None, transcript) when
    the step budget runs out before a final answer."""
    transcript = [{"type": "task", "content": task}]
    for _ in range(max_steps):
        action = model(transcript)
        transcript.append(action)

        if action["type"] == "final":
            return action["answer"], transcript

        # Anything that isn't final is treated as a tool-call attempt.
        errors = validate_tool_call(action, tools)
        if errors:
            transcript.append(
                {
                    "type": "tool_error",
                    "name": action.get("name"),
                    "error": "; ".join(errors),
                }
            )
            continue

        name = action["name"]
        try:
            result = tools[name](**action["arguments"])
        except Exception as e:  # errors are data: feed them back, never crash
            transcript.append({"type": "tool_error", "name": name, "error": str(e)})
        else:
            transcript.append({"type": "tool_result", "name": name, "result": result})

    return None, transcript
