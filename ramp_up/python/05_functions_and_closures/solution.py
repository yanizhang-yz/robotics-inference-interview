"""
Functions and Closures — reference solutions.

The theme: in Python a function is an ordinary object. Everything Java does
with functional interfaces, overloading, and Comparators falls out of three
facts — functions are values, closures capture their scope, and defaults are
evaluated once at def time.
"""

import functools
from collections.abc import Callable
from typing import Any


def apply_n_times(f: Callable[[Any], Any], n: int, x: Any) -> Any:
    # No Function<T,R> wrapper: f is a value, calling it is just f(x).
    for _ in range(n):
        x = f(x)
    return x


def make_multiplier(k: float) -> Callable[[float], float]:
    # The lambda closes over k — no "effectively final" restriction,
    # and callers invoke it directly: triple(5), not triple.apply(5).
    return lambda x: x * k


def make_counter() -> Callable[[], int]:
    count = 0

    def counter() -> int:
        # Without `nonlocal`, `count += 1` would make count a NEW local
        # variable and blow up with UnboundLocalError. nonlocal says
        # "rebind the count from the enclosing scope".
        nonlocal count
        count += 1
        return count

    return counter


def safe_divide(a: float, b: float, default: float | None = None) -> float | None:
    # One function + a default value replaces Java's overload pair.
    # EAFP: try it, catch the failure — no `if b == 0` pre-check.
    try:
        return a / b
    except ZeroDivisionError:
        return default


def append_to(item: Any, target: list | None = None) -> list:
    # The None-sentinel idiom. `target=[]` in the signature would create
    # ONE list at def time, shared by every call (see README).
    if target is None:
        target = []
    target.append(item)
    return target


def describe_call(*args: Any, **kwargs: Any) -> str:
    # *args arrives as a tuple, **kwargs as an insertion-ordered dict.
    return f"args={args} kwargs={kwargs}"


def compose(*funcs: Callable[[Any], Any]) -> Callable[[Any], Any]:
    def composed(x: Any) -> Any:
        # Right-to-left, like math notation: compose(f, g)(x) == f(g(x)).
        for f in reversed(funcs):
            x = f(x)
        return x

    return composed


def memoize(f: Callable[..., Any]) -> Callable[..., Any]:
    cache: dict[tuple, Any] = {}

    # @functools.wraps copies f's __name__/__doc__ onto the wrapper —
    # without it every decorated function reports itself as "wrapper".
    @functools.wraps(f)
    def wrapper(*args: Any) -> Any:
        if args not in cache:
            cache[args] = f(*args)
        return cache[args]

    return wrapper


def call_with_retry(f: Callable[[], Any], attempts: int) -> Any:
    if attempts < 1:
        raise ValueError(f"attempts must be >= 1, got {attempts}")
    last_exc: Exception | None = None
    for _ in range(attempts):
        try:
            return f()
        except Exception as exc:  # EAFP: attempt, catch, retry
            last_exc = exc
    raise last_exc  # bare-name raise keeps the original traceback


def sort_by(records: list[dict], *fields: str) -> list[dict]:
    # A tuple key IS Comparator.comparing(...).thenComparing(...):
    # tuples compare element by element, left to right.
    # sorted() returns a new list — the input is untouched.
    return sorted(records, key=lambda r: tuple(r[field] for field in fields))
