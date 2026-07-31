import pytest

from solution import run_agent, validate_tool_call


def scripted(actions):
    """Model that ignores the transcript and plays back a fixed script."""
    it = iter(actions)
    return lambda transcript: next(it)


class TestToolCallsThenFinal:
    def test_calculator_twice_then_final(self):
        tools = {"calculator": lambda a, b: a + b}
        model = scripted(
            [
                {"type": "tool_call", "name": "calculator", "arguments": {"a": 2, "b": 3}},
                {"type": "tool_call", "name": "calculator", "arguments": {"a": 5, "b": 10}},
                {"type": "final", "answer": "5 and 15"},
            ]
        )
        answer, transcript = run_agent(model, tools, "add 2+3, then 5+10")

        assert answer == "5 and 15"
        assert [entry["type"] for entry in transcript] == [
            "task",
            "tool_call",
            "tool_result",
            "tool_call",
            "tool_result",
            "final",
        ]
        assert transcript[0] == {"type": "task", "content": "add 2+3, then 5+10"}
        assert transcript[2] == {"type": "tool_result", "name": "calculator", "result": 5}
        assert transcript[4] == {"type": "tool_result", "name": "calculator", "result": 15}

    def test_empty_tools_with_immediate_final(self):
        model = scripted([{"type": "final", "answer": "nothing to do"}])
        answer, transcript = run_agent(model, {}, "just answer")
        assert answer == "nothing to do"
        assert [entry["type"] for entry in transcript] == ["task", "final"]


class TestErrorRecovery:
    def test_unknown_tool_appends_error_and_loop_continues(self):
        tools = {"search": lambda query: f"results for {query}"}

        def model(transcript):
            last = transcript[-1]
            if last["type"] == "task":  # first try: misspelled tool name
                return {"type": "tool_call", "name": "serach", "arguments": {"query": "kv"}}
            if last["type"] == "tool_error":  # read the error, correct the name
                return {"type": "tool_call", "name": "search", "arguments": {"query": "kv"}}
            return {"type": "final", "answer": last["result"]}

        answer, transcript = run_agent(model, tools, "look up kv")

        assert answer == "results for kv"
        assert [entry["type"] for entry in transcript] == [
            "task",
            "tool_call",
            "tool_error",
            "tool_call",
            "tool_result",
            "final",
        ]
        assert "unknown tool" in transcript[2]["error"]

    def test_tool_raising_becomes_tool_error_not_crash(self):
        def brittle(x):
            raise ValueError(f"bad input: {x}")

        model = scripted(
            [
                {"type": "tool_call", "name": "brittle", "arguments": {"x": 42}},
                {"type": "final", "answer": "recovered"},
            ]
        )
        answer, transcript = run_agent(model, {"brittle": brittle}, "poke the tool")

        assert answer == "recovered"
        error_entry = transcript[2]
        assert error_entry["type"] == "tool_error"
        assert error_entry["name"] == "brittle"
        assert error_entry["error"] == "bad input: 42"


class TestStepBudget:
    def test_max_steps_returns_none_with_exact_model_turns(self):
        tools = {"noop": lambda: None}

        def looping_model(transcript):
            return {"type": "tool_call", "name": "noop", "arguments": {}}

        answer, transcript = run_agent(looping_model, tools, "spin", max_steps=3)

        assert answer is None
        model_turns = [e for e in transcript if e["type"] == "tool_call"]
        assert len(model_turns) == 3  # exactly max_steps model calls
        # task + 3 * (tool_call + tool_result)
        assert len(transcript) == 7


class TestTwoToolScenario:
    def test_lookup_then_compute(self):
        tools = {
            "lookup": lambda key: {"wheel_radius": 3.0}[key],
            "scale": lambda x, factor: x * factor,
        }

        def model(transcript):
            last = transcript[-1]
            if last["type"] == "task":
                return {"type": "tool_call", "name": "lookup", "arguments": {"key": "wheel_radius"}}
            if last["type"] == "tool_result" and last["name"] == "lookup":
                # Feed the first tool's result into the second tool.
                return {
                    "type": "tool_call",
                    "name": "scale",
                    "arguments": {"x": last["result"], "factor": 2.0},
                }
            return {"type": "final", "answer": last["result"]}

        answer, transcript = run_agent(model, tools, "double the wheel radius")

        assert answer == 6.0
        assert [entry["type"] for entry in transcript] == [
            "task",
            "tool_call",
            "tool_result",
            "tool_call",
            "tool_result",
            "final",
        ]
        assert transcript[4]["result"] == 6.0


class TestValidateToolCall:
    TOOLS = {"search": lambda query: query}

    def test_well_formed_call_has_no_errors(self):
        call = {"type": "tool_call", "name": "search", "arguments": {"query": "hi"}}
        assert validate_tool_call(call, self.TOOLS) == []

    def test_missing_name(self):
        errors = validate_tool_call({"type": "tool_call", "arguments": {}}, self.TOOLS)
        assert errors == ["missing 'name'"]

    def test_unknown_tool(self):
        call = {"type": "tool_call", "name": "grasp", "arguments": {}}
        errors = validate_tool_call(call, self.TOOLS)
        assert len(errors) == 1
        assert "unknown tool" in errors[0]
        assert "grasp" in errors[0]

    def test_missing_arguments(self):
        errors = validate_tool_call({"type": "tool_call", "name": "search"}, self.TOOLS)
        assert errors == ["missing 'arguments'"]

    def test_arguments_not_a_dict(self):
        call = {"type": "tool_call", "name": "search", "arguments": ["hi"]}
        assert validate_tool_call(call, self.TOOLS) == ["'arguments' must be a dict"]

    def test_multiple_errors_collected(self):
        errors = validate_tool_call({}, self.TOOLS)
        assert len(errors) == 2
        assert any("name" in e for e in errors)
        assert any("arguments" in e for e in errors)


class TestMalformedCallFedBackToModel:
    def test_missing_arguments_becomes_tool_error_and_model_recovers(self):
        tools = {"search": lambda query: f"found {query}"}

        def model(transcript):
            last = transcript[-1]
            if last["type"] == "task":  # malformed: no arguments key
                return {"type": "tool_call", "name": "search"}
            if last["type"] == "tool_error":
                return {"type": "tool_call", "name": "search", "arguments": {"query": "x"}}
            return {"type": "final", "answer": last["result"]}

        answer, transcript = run_agent(model, tools, "search for x")

        assert answer == "found x"
        assert transcript[2]["type"] == "tool_error"
        assert "arguments" in transcript[2]["error"]
