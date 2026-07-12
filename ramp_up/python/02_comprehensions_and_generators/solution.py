"""
Comprehensions and Generators — reference solutions.

The recurring theme: what Java Streams expresses as a pipeline with a
terminal collector, Python expresses as a comprehension (eager, bracket
picks the container) or a generator (lazy, produced on demand).
"""

import itertools
from collections.abc import Callable, Iterable, Iterator
from typing import Any


def squares_of_evens(nums: list[int]) -> list[int]:
    # filter (the trailing if) + map (the leading expression) + collect
    # (the brackets), all in one line.
    return [n * n for n in nums if n % 2 == 0]


def word_length_map(words: list[str]) -> dict[str, int]:
    return {w: len(w) for w in words}


def unique_first_letters(words: list[str]) -> set[str]:
    # "if w" filters empty strings — empty containers are falsy in Python.
    return {w[0].lower() for w in words if w}


def transpose(matrix: list[list]) -> list[list]:
    # * unpacks rows into zip's arguments; zip staples them column-wise.
    return [list(col) for col in zip(*matrix)]


def flatten_matrix(matrix: list[list]) -> list:
    # for-clauses read in nested-loop order: outer loop first.
    return [x for row in matrix for x in row]


def running_totals(nums: list[int]) -> list[int]:
    # accumulate is lazy; list() materializes it.
    return list(itertools.accumulate(nums))


def countdown(n: int) -> Iterator[int]:
    # The presence of `yield` makes this a generator function: calling it
    # runs NO body code, it just builds an iterator. State (i) is kept
    # between next() calls — no hasNext()/next() boilerplate.
    for i in range(n, 0, -1):
        yield i


def first_matching(
    iterable: Iterable,
    predicate: Callable[[Any], bool],
    default: Any = None,
) -> Any:
    # The generator expression is lazy, so predicate stops running at the
    # first match — same short-circuiting as findFirst().
    return next((x for x in iterable if predicate(x)), default)


def adjacent_pairs(lst: list) -> list[tuple]:
    # Self-zip against the off-by-one slice; zip stops at the shorter one.
    return list(zip(lst, lst[1:]))


def all_products(colors: list, sizes: list) -> list[tuple]:
    return list(itertools.product(colors, sizes))
