"""
Comprehensions and Generators — YOUR ATTEMPT

Write your code here, then run the tests against it:
    PRACTICE=1 uv run pytest ramp_up/python/02_comprehensions_and_generators -v

Peek at solution.py only after you've tried. Each docstring names the Java
Streams construct you'd normally reach for and the Python idiom that
replaces it.
"""

from collections.abc import Callable, Iterable, Iterator
from typing import Any


def squares_of_evens(nums: list[int]) -> list[int]:
    """
    Return the squares of the even numbers, preserving order.

    JAVA:   nums.stream().filter(n -> n % 2 == 0).map(n -> n * n)
                .collect(Collectors.toList());
    PYTHON: one list comprehension does filter + map + collect:
            [n * n for n in nums if n % 2 == 0]
    """
    raise NotImplementedError


def word_length_map(words: list[str]) -> dict[str, int]:
    """
    Return a dict mapping each word to its length.

    JAVA:   words.stream().collect(Collectors.toMap(w -> w, String::length));
    PYTHON: dict comprehension: {w: len(w) for w in words}. Duplicate keys
            silently keep the LAST value (toMap would throw).
    """
    raise NotImplementedError


def unique_first_letters(words: list[str]) -> set[str]:
    """
    Return the set of lowercased first letters. Ignore empty strings.

    JAVA:   words.stream().filter(w -> !w.isEmpty())
                .map(w -> Character.toLowerCase(w.charAt(0)))
                .collect(Collectors.toSet());
    PYTHON: set comprehension — curly braces, no key-colon:
            {w[0].lower() for w in words if w}
            (an empty string is falsy, so "if w" filters it out)
    """
    raise NotImplementedError


def transpose(matrix: list[list]) -> list[list]:
    """
    Return the transpose of a rectangular matrix (list of rows).

    JAVA:   allocate int[cols][rows] and copy with two index loops.
    PYTHON: zip(*matrix) — the * unpacks the rows as separate arguments and
            zip pairs them up column-wise. zip yields tuples, so convert:
            [list(col) for col in zip(*matrix)]
    """
    raise NotImplementedError


def flatten_matrix(matrix: list[list]) -> list:
    """
    Flatten a list of rows into one flat list, row-major order.

    JAVA:   matrix.stream().flatMap(List::stream).collect(toList());
    PYTHON: nested comprehension; the for-clauses appear in the SAME order
            as the equivalent nested loops (outer first):
            [x for row in matrix for x in row]
    """
    raise NotImplementedError


def running_totals(nums: list[int]) -> list[int]:
    """
    Return the running (prefix) sums: [1, 2, 3] -> [1, 3, 6].

    JAVA:   a loop with a mutable accumulator, or Arrays.parallelPrefix.
    PYTHON: the standard library already has it:
            list(itertools.accumulate(nums))
    """
    raise NotImplementedError


def countdown(n: int) -> Iterator[int]:
    """
    GENERATOR yielding n, n-1, ..., 1. countdown(0) yields nothing.

    JAVA:   implement Iterator<Integer> with a current-value field and
            hasNext()/next() methods.
    PYTHON: a function with `yield` IS an iterator — Python keeps the state
            between calls for you. Values are produced lazily, one per
            next() call; the caller can list() it or pull with next().
    """
    raise NotImplementedError


def first_matching(
    iterable: Iterable,
    predicate: Callable[[Any], bool],
    default: Any = None,
) -> Any:
    """
    Return the first element satisfying predicate, or default if none does.
    Must not consume the iterable past the first match.

    JAVA:   stream.filter(predicate).findFirst().orElse(default);
    PYTHON: next() with a generator expression and a default:
            next((x for x in iterable if predicate(x)), default)
            The genexpr is lazy, so evaluation stops at the first hit.
    """
    raise NotImplementedError


def adjacent_pairs(lst: list) -> list[tuple]:
    """
    Return consecutive pairs: [1, 2, 3] -> [(1, 2), (2, 3)].
    Lists shorter than 2 elements produce [].

    JAVA:   for (int i = 0; i < list.size() - 1; i++)
                pairs.add(Pair.of(list.get(i), list.get(i + 1)));
    PYTHON: zip the list against itself shifted by one:
            list(zip(lst, lst[1:]))
            zip stops at the shorter argument, so the bounds work out.
    """
    raise NotImplementedError


def all_products(colors: list, sizes: list) -> list[tuple]:
    """
    Return every (color, size) combination, colors varying slowest —
    the Cartesian product.

    JAVA:   two nested for-loops appending new Pair<>(color, size).
    PYTHON: list(itertools.product(colors, sizes)) — or the nested
            comprehension [(c, s) for c in colors for s in sizes].
    """
    raise NotImplementedError
