"""
Agent Tool Loop — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/07_llm_and_agents/003_agent_tool_loop -v

Peek at solution.py only after you've tried.
"""


def validate_tool_call(call: dict, tools: dict) -> list[str]:
    """
    Return a list of error strings for a tool-call dict (empty = well-formed).

    Checks, each contributing one message:
    1. "name" key missing            -> "missing 'name'"
    2. name present but not in tools -> "unknown tool: <name>"
    3. "arguments" key missing       -> "missing 'arguments'"
    4. arguments present, not a dict -> "'arguments' must be a dict"

    Shape checks only — never call the tool. A call can collect several
    errors at once (e.g. {} yields two).
    """
    # TODO: implement
    raise NotImplementedError


def run_agent(model, tools: dict, task: str, max_steps: int = 10):
    """
    Run the agent loop; return (answer, transcript).

    Plan:
    1. transcript = [{"type": "task", "content": task}]
    2. Loop at most max_steps times (each iteration = one model call):
       a. action = model(transcript); append action to the transcript.
       b. action["type"] == "final" -> return (action["answer"], transcript).
       c. Otherwise it's a tool call. errors = validate_tool_call(action, tools);
          if errors: append {"type": "tool_error", "name": action.get("name"),
          "error": "; ".join(errors)} and continue — the model reads the error
          next iteration and can recover.
       d. Valid call: run tools[name](**arguments) in a try/except Exception.
          Success -> append {"type": "tool_result", "name": name, "result": r}.
          Exception e -> append {"type": "tool_error", "name": name,
          "error": str(e)}. The loop itself must NEVER crash.
    3. Budget exhausted with no final answer -> return (None, transcript).
    """
    # TODO: implement
    raise NotImplementedError
