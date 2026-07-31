"""
Functions and Closures — YOUR ATTEMPT

Write your implementations here, then run the tests against them:
    PRACTICE=1 uv run pytest ramp_up/python/05_functions_and_closures -v

Peek at solution.py only after you've tried each drill.
"""
import functools
from collections.abc import Callable
from typing import Any


def apply_n_times(f: Callable[[Any], Any], n: int, x: Any) -> Any:
    """
    Apply f to x, n times: f(f(...f(x))). n == 0 returns x unchanged.

    HINT: functions are ordinary values; just call f(x) in a loop.
    """
    raise NotImplementedError


def make_multiplier(k: float) -> Callable[[float], float]:
    """
    Return a function that multiplies its argument by k.
    make_multiplier(3)(5) == 15.

    HINT: return a lambda (or inner def) that closes over k.
    """
    raise NotImplementedError


def make_counter() -> Callable[[], int]:
    """
    Return a function that returns 1, 2, 3, ... on successive calls.
    Each counter from make_counter() is independent.

    HINT: a closure CAN rebind outer state — but only if you declare
          `nonlocal count` first. Forgetting nonlocal (and getting
          UnboundLocalError) is THE classic closure gotcha.
    """
    raise NotImplementedError


def safe_divide(a: float, b: float, default: float | None = None) -> float | None:
    """
    Return a / b, or `default` if the division fails (b == 0).

    HINT: one function with a default parameter value; EAFP style —
          try the division and catch ZeroDivisionError.
    """
    raise NotImplementedError


def append_to(item: Any, target: list | None = None) -> list:
    """
    Append item to target and return it. If target is omitted, use a NEW
    empty list — a fresh one on every call.

    HINT: `target=[]` would be evaluated ONCE at def time and shared across
          calls (see README). Use the None-sentinel idiom: default to None,
          then create the list inside the body.
    """
    raise NotImplementedError


def describe_call(*args: Any, **kwargs: Any) -> str:
    """
    Return a string describing the call, exactly like:
        describe_call(1, 2, x=3)  ->  "args=(1, 2) kwargs={'x': 3}"
        describe_call()           ->  "args=() kwargs={}"

    HINT: *args collects extra positionals into a tuple, **kwargs collects
          keyword arguments into a dict (insertion-ordered).
    """
    raise NotImplementedError


def compose(*funcs: Callable[[Any], Any]) -> Callable[[Any], Any]:
    """
    Compose functions right-to-left: compose(f, g, h)(x) == f(g(h(x))).
    compose() with no functions returns the identity function.

    HINT: return a new function that loops over reversed(funcs), threading
          the value through. Functions building functions.
    """
    raise NotImplementedError


def memoize(f: Callable[..., Any]) -> Callable[..., Any]:
    """
    Decorator: cache f's results by its positional arguments, so repeated
    calls with the same args invoke f only once. Use @functools.wraps on the
    wrapper so the decorated function keeps f's __name__ and docstring.

    Usage:
        @memoize
        def slow(x): ...

    HINT: a decorator — a function that takes f and returns a wrapper
          closing over a cache dict. (In real code: functools.lru_cache;
          here you build it yourself to learn the mechanics.)
    """

    raise NotImplementedError


def call_with_retry(f: Callable[[], Any], attempts: int) -> Any:
    """
    Call f() up to `attempts` times, returning the first successful result.
    If every attempt raises, re-raise the LAST exception.
    attempts < 1 raises ValueError.

    HINT: a for-loop with try/except, stashing the exception; after the loop
          a bare `raise last_exc` re-raises with the traceback intact.
    """
    raise NotImplementedError


def sort_by(records: list[dict], *fields: str) -> list[dict]:
    """
    Return a NEW list of dicts sorted ascending by the given fields, in order
    of significance: sort_by(rows, "priority", "name") sorts by priority,
    breaking ties by name. Do not mutate the input list.

    HINT: sorted(records, key=lambda r: tuple-of-field-values) — tuples
          compare element by element, so earlier fields dominate.
    """
    raise NotImplementedError
