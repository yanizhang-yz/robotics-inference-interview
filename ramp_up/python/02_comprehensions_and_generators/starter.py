"""
Comprehensions and Generators — YOUR ATTEMPT

Write your code here, then run the tests against it:
    PRACTICE=1 uv run pytest ramp_up/python/02_comprehensions_and_generators -v

Peek at solution.py only after you've tried. Each docstring names the
Python idiom to reach for.
"""

from collections.abc import Callable, Iterable, Iterator
import itertools
from typing import Any


def squares_of_evens(nums: list[int]) -> list[int]:
    """
    Return the squares of the even numbers, preserving order.

    HINT: one list comprehension does filter + map + collect:
          [n * n for n in nums if n % 2 == 0]
    """
    return [ n * n for n in nums if n % 2 == 0]


def word_length_map(words: list[str]) -> dict[str, int]:
    """
    Return a dict mapping each word to its length.

    HINT: dict comprehension: {w: len(w) for w in words}. Duplicate keys
          silently keep the LAST value — no error, no warning.
    """
    return { w: len(w) for w in words}


def unique_first_letters(words: list[str]) -> set[str]:
    """
    Return the set of lowercased first letters. Ignore empty strings.

    HINT: set comprehension — curly braces, no key-colon:
          {w[0].lower() for w in words if w}
          (an empty string is falsy, so "if w" filters it out)
    """
    return {w[0].lower() for w in words if w}


def transpose(matrix: list[list]) -> list[list]:
    """
    Return the transpose of a rectangular matrix (list of rows).

    HINT: zip(*matrix) — the * unpacks the rows as separate arguments and
          zip pairs them up column-wise. zip yields tuples, so convert:
          [list(col) for col in zip(*matrix)]
    """
    return [list(col) for col in zip(*matrix)]


def flatten_matrix(matrix: list[list]) -> list:
    """
    Flatten a list of rows into one flat list, row-major order.

    HINT: nested comprehension; the for-clauses appear in the SAME order
          as the equivalent nested loops (outer first):
          [x for row in matrix for x in row]
    """
    return [x for row in matrix for x in row]


def running_totals(nums: list[int]) -> list[int]:
    """
    Return the running (prefix) sums: [1, 2, 3] -> [1, 3, 6].

    HINT: the standard library already has it:
          list(itertools.accumulate(nums))
    """
    return list(itertools.accumulate(nums))


def countdown(n: int) -> Iterator[int]:
    """
    GENERATOR yielding n, n-1, ..., 1. countdown(0) yields nothing.

    HINT: a function with `yield` IS an iterator — Python keeps the state
          between calls for you. Values are produced lazily, one per
          next() call; the caller can list() it or pull with next().
    """
    for i in range(n, 0, -1):
        yield i


def first_matching(
    iterable: Iterable,
    predicate: Callable[[Any], bool],
    default: Any = None,
) -> Any:
    """
    Return the first element satisfying predicate, or default if none does.
    Must not consume the iterable past the first match.

    HINT: next() with a generator expression and a default:
          next((x for x in iterable if predicate(x)), default)
          The genexpr is lazy, so evaluation stops at the first hit.
    """
    return next((x for x in iterable if predicate(x)), default)


def adjacent_pairs(lst: list) -> list[tuple]:
    """
    Return consecutive pairs: [1, 2, 3] -> [(1, 2), (2, 3)].
    Lists shorter than 2 elements produce [].

    HINT: zip the list against itself shifted by one:
          list(zip(lst, lst[1:]))
          zip stops at the shorter argument, so the bounds work out.
    """
    return list(zip(lst, lst[1:]))


def all_products(colors: list, sizes: list) -> list[tuple]:
    """
    Return every (color, size) combination, colors varying slowest —
    the Cartesian product.

    HINT: list(itertools.product(colors, sizes)) — or the nested
          comprehension [(c, s) for c in colors for s in sizes].
    """
    return list(itertools.product(colors, sizes))
