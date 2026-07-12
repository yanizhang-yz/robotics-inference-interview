import pytest
from solution import (
    append_to,
    apply_n_times,
    call_with_retry,
    compose,
    describe_call,
    make_counter,
    make_multiplier,
    memoize,
    safe_divide,
    sort_by,
)


class TestApplyNTimes:
    def test_repeated_application(self):
        assert apply_n_times(lambda x: x * 2, 3, 1) == 8

    def test_zero_times_returns_input_unchanged(self):
        assert apply_n_times(lambda x: x + 1, 0, 42) == 42

    def test_works_with_named_functions(self):
        assert apply_n_times(str.upper, 1, "hi") == "HI"


class TestMakeMultiplier:
    def test_basic_multiplication(self):
        triple = make_multiplier(3)
        assert triple(5) == 15

    def test_each_closure_captures_its_own_k(self):
        double = make_multiplier(2)
        halve = make_multiplier(0.5)
        assert double(10) == 20
        assert halve(10) == 5.0

    def test_zero_multiplier(self):
        assert make_multiplier(0)(999) == 0


class TestMakeCounter:
    def test_counts_up_from_one(self):
        counter = make_counter()
        assert counter() == 1
        assert counter() == 2
        assert counter() == 3

    def test_counters_are_independent(self):
        a = make_counter()
        b = make_counter()
        a()
        a()
        assert b() == 1  # b has its own captured state
        assert a() == 3


class TestSafeDivide:
    def test_normal_division(self):
        assert safe_divide(10, 4) == 2.5

    def test_divide_by_zero_returns_default_none(self):
        assert safe_divide(1, 0) is None

    def test_divide_by_zero_with_explicit_default(self):
        assert safe_divide(1, 0, default=-1.0) == -1.0

    def test_default_ignored_when_division_succeeds(self):
        assert safe_divide(9, 3, default=-1.0) == 3.0


class TestAppendTo:
    def test_appends_to_given_list(self):
        existing = [1, 2]
        result = append_to(3, existing)
        assert result == [1, 2, 3]
        assert result is existing  # mutates and returns the caller's list

    def test_fresh_list_when_omitted(self):
        assert append_to("a") == ["a"]

    def test_no_shared_state_between_calls(self):
        # THE trap: with target=[] in the signature, the second call
        # would return ["x", "y"].
        first = append_to("x")
        second = append_to("y")
        assert first == ["x"]
        assert second == ["y"]
        assert first is not second


class TestDescribeCall:
    def test_args_and_kwargs(self):
        assert describe_call(1, 2, x=3) == "args=(1, 2) kwargs={'x': 3}"

    def test_no_arguments(self):
        assert describe_call() == "args=() kwargs={}"

    def test_kwargs_only(self):
        assert describe_call(a=1, b=2) == "args=() kwargs={'a': 1, 'b': 2}"

    def test_single_positional_shows_tuple(self):
        assert describe_call(7) == "args=(7,) kwargs={}"


class TestCompose:
    def test_applies_right_to_left(self):
        add_one = lambda x: x + 1
        double = lambda x: x * 2
        # compose(f, g)(x) == f(g(x)): double first, then add one.
        assert compose(add_one, double)(5) == 11
        assert compose(double, add_one)(5) == 12

    def test_three_functions(self):
        f = compose(str, lambda x: x + 1, lambda x: x * 10)
        assert f(4) == "41"

    def test_empty_compose_is_identity(self):
        assert compose()(99) == 99


class TestMemoize:
    def test_returns_correct_results(self):
        @memoize
        def square(x):
            return x * x

        assert square(4) == 16
        assert square(5) == 25

    def test_underlying_function_called_once_per_args(self):
        calls = []  # spy: records every real invocation

        @memoize
        def tracked(x):
            calls.append(x)
            return x * 10

        assert tracked(3) == 30
        assert tracked(3) == 30
        assert tracked(3) == 30
        assert calls == [3]  # cached after the first call

    def test_distinct_args_each_computed(self):
        calls = []

        @memoize
        def tracked(a, b):
            calls.append((a, b))
            return a + b

        assert tracked(1, 2) == 3
        assert tracked(2, 1) == 3
        assert tracked(1, 2) == 3
        assert calls == [(1, 2), (2, 1)]

    def test_wraps_preserves_metadata(self):
        @memoize
        def documented(x):
            """Original docstring."""
            return x

        assert documented.__name__ == "documented"
        assert documented.__doc__ == "Original docstring."


class TestCallWithRetry:
    def test_success_on_first_attempt(self):
        assert call_with_retry(lambda: "ok", 3) == "ok"

    def test_succeeds_after_failures(self):
        attempts = []

        def flaky():
            attempts.append(1)
            if len(attempts) < 3:
                raise RuntimeError(f"boom {len(attempts)}")
            return "recovered"

        assert call_with_retry(flaky, 5) == "recovered"
        assert len(attempts) == 3  # stops retrying once it succeeds

    def test_reraises_last_exception_when_all_fail(self):
        attempts = []

        def always_fails():
            attempts.append(1)
            raise RuntimeError(f"boom {len(attempts)}")

        with pytest.raises(RuntimeError, match="boom 2"):
            call_with_retry(always_fails, 2)
        assert len(attempts) == 2

    def test_zero_attempts_is_an_error(self):
        with pytest.raises(ValueError):
            call_with_retry(lambda: "ok", 0)


class TestSortBy:
    def test_single_field(self):
        rows = [{"name": "b"}, {"name": "a"}, {"name": "c"}]
        assert sort_by(rows, "name") == [{"name": "a"}, {"name": "b"}, {"name": "c"}]

    def test_multiple_fields_break_ties_in_order(self):
        rows = [
            {"priority": 2, "name": "deploy"},
            {"priority": 1, "name": "test"},
            {"priority": 1, "name": "build"},
        ]
        assert sort_by(rows, "priority", "name") == [
            {"priority": 1, "name": "build"},
            {"priority": 1, "name": "test"},
            {"priority": 2, "name": "deploy"},
        ]

    def test_input_not_mutated(self):
        rows = [{"n": 2}, {"n": 1}]
        result = sort_by(rows, "n")
        assert rows == [{"n": 2}, {"n": 1}]  # original order preserved
        assert result is not rows

    def test_empty_list(self):
        assert sort_by([], "anything") == []
