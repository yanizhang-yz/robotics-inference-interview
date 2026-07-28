"""
Collections and Slicing — YOUR ATTEMPT

Write your code here, then run the tests against it:
    PRACTICE=1 uv run pytest ramp_up/python/01_collections_and_slicing -v

Peek at solution.py only after you've tried. Each docstring names the
Python idiom to reach for.
"""


def rotate_left(lst: list, k: int) -> list:
    """
    Return a new list rotated left by k positions. k may exceed len(lst);
    an empty list stays empty.

    HINT: slice and concatenate: lst[k:] + lst[:k], with k reduced modulo
          len(lst) first. Slices never throw, so no bounds checks needed.
    """
    if not lst:
        return []

    k = k % len(lst)
    return lst[k:] + lst[:k]

def last_n(lst: list, n: int) -> list:
    """
    Return the last n items as a new list. n == 0 returns [].

    HINT: negative slicing: lst[-n:]. Careful — when n == 0, lst[-0:] is
          lst[0:] (the whole list), so n == 0 needs explicit handling.
    """
    if n == 0:
        return []
    if n < 0:
        raise ValueError("n must be non-negative")
    return lst[-n:]


def every_other(lst: list) -> list:
    """
    Return items at even indices (0, 2, 4, ...) as a new list.

    HINT: step slicing: lst[::2]. The third slice component is the stride.
    """
    return lst[::2]

def reverse_copy(lst: list) -> list:
    """
    Return a reversed copy; the input list must not be modified.

    HINT: lst[::-1] — a negative stride walks backwards and, like every
          slice, returns a NEW list. (lst.reverse() is the mutating twin
          and returns None.)
    """
    return lst[::-1]


def swap_ends(lst: list) -> list:
    """
    Return a copy with the first and last elements swapped.
    Lists shorter than 2 elements are returned as an (unmodified) copy.

    HINT: tuple unpacking, no temp variable:
          out[0], out[-1] = out[-1], out[0]
          (Make the copy first — e.g. list(lst) — so the input is untouched.)
    """
    if len(lst) < 2:
        return list(lst)
    out = list(lst)
    out[0], out[-1] = out[-1], out[0]
    return out


def interleave(a: list, b: list) -> list:
    """
    Return [a[0], b[0], a[1], b[1], ...], stopping at the shorter input.

    HINT: zip(a, b) pairs elements and stops at the shorter input for
          free; flatten the pairs with a nested comprehension:
          [x for pair in zip(a, b) for x in pair]
    """
    return list(x for pair in zip(a, b) for x in pair)


def sort_by_length_then_alpha(words: list[str]) -> list[str]:
    """
    Return words sorted by length, ties broken alphabetically. Non-mutating.

    HINT: sorted(words, key=lambda w: (len(w), w)) — the key function maps
          each element to a tuple, and tuples compare field by field, so a
          tuple key IS a multi-level sort.
    """
    return sorted(words, key=lambda w: (len(w), w))



def top_k_smallest(nums: list, k: int) -> list:
    """
    Return the k smallest numbers in ascending order (k > len is fine —
    return everything, sorted).

    HINT: sorted(nums)[:k]. sorted() always returns a new list, and a
          slice past the end just clamps — no min() needed.
    """
    return sorted(nums)[:k]


def chunk(lst: list, size: int) -> list[list]:
    """
    Split lst into consecutive chunks of the given size; the last chunk may
    be shorter. chunk([], size) -> [].

    HINT: one comprehension — slices clamp at the end automatically:
          [lst[i:i + size] for i in range(0, len(lst), size)]
    """
    return [lst[i:i+size] for i in range(0, len(lst), size)]

def flatten_one_level(nested: list[list]) -> list:
    """
    Flatten a list of lists into a single list (one level deep only).

    HINT: nested comprehension, loops read left to right like the
          equivalent for-loops: [x for sub in nested for x in sub]
    """
    return [ x for lst in nested for x in lst]
