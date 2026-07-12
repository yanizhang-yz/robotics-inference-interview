"""
Collections and Slicing — reference solutions.

Every function here returns a NEW list. That is the Pythonic default and the
opposite of Java's Collections.reverse / Collections.sort habit of mutating
the argument.
"""


def rotate_left(lst: list, k: int) -> list:
    # k % len(lst) handles k > len; slicing an empty list is guarded because
    # % 0 would raise ZeroDivisionError.
    if not lst:  # idiomatic emptiness check — not len(lst) == 0
        return []
    k %= len(lst)
    return lst[k:] + lst[:k]


def last_n(lst: list, n: int) -> list:
    # lst[-0:] == lst[0:] == the whole list, so n == 0 is the one case
    # negative slicing can't express.
    return lst[-n:] if n > 0 else []


def every_other(lst: list) -> list:
    return lst[::2]


def reverse_copy(lst: list) -> list:
    # Slice with stride -1: copies AND reverses in one expression.
    return lst[::-1]


def swap_ends(lst: list) -> list:
    out = list(lst)  # list(x) is the idiomatic shallow copy (like new ArrayList<>(x))
    if len(out) >= 2:
        out[0], out[-1] = out[-1], out[0]  # tuple unpacking: no tmp variable
    return out


def interleave(a: list, b: list) -> list:
    # zip stops at the shorter input — no Math.min bookkeeping.
    return [x for pair in zip(a, b) for x in pair]


def sort_by_length_then_alpha(words: list[str]) -> list[str]:
    # A tuple key is a comparator chain: compare len first, then the word.
    return sorted(words, key=lambda w: (len(w), w))


def top_k_smallest(nums: list, k: int) -> list:
    # sorted() copies; the slice clamps if k > len(nums).
    return sorted(nums)[:k]


def chunk(lst: list, size: int) -> list[list]:
    # The final slice lst[i:i+size] clamps at the end — no bounds math.
    return [lst[i : i + size] for i in range(0, len(lst), size)]


def flatten_one_level(nested: list[list]) -> list:
    # Read the two for-clauses left to right, exactly like nested loops.
    return [x for sub in nested for x in sub]
