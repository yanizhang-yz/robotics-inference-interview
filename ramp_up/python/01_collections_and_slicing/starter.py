"""
Collections and Slicing — YOUR ATTEMPT

Write your code here, then run the tests against it:
    PRACTICE=1 uv run pytest ramp_up/python/01_collections_and_slicing -v

Peek at solution.py only after you've tried. Each docstring names the Java
tool you'd normally reach for and the Python idiom that replaces it.
"""


def rotate_left(lst: list, k: int) -> list:
    """
    Return a new list rotated left by k positions. k may exceed len(lst);
    an empty list stays empty.

    JAVA:   Collections.rotate(list, -k) — mutates in place — or manual index
            arithmetic with (i + k) % n.
    PYTHON: slice and concatenate: lst[k:] + lst[:k], with k reduced modulo
            len(lst) first. Slices never throw, so no bounds checks needed.
    """
    raise NotImplementedError


def last_n(lst: list, n: int) -> list:
    """
    Return the last n items as a new list. n == 0 returns [].

    JAVA:   list.subList(list.size() - n, list.size()) — and an
            IndexOutOfBoundsException if you get the math wrong.
    PYTHON: negative slicing: lst[-n:]. Careful — when n == 0, lst[-0:] is
            lst[0:] (the whole list), so n == 0 needs explicit handling.
    """
    raise NotImplementedError


def every_other(lst: list) -> list:
    """
    Return items at even indices (0, 2, 4, ...) as a new list.

    JAVA:   for (int i = 0; i < a.length; i += 2) result.add(a[i]);
    PYTHON: step slicing: lst[::2]. The third slice component is the stride.
    """
    raise NotImplementedError


def reverse_copy(lst: list) -> list:
    """
    Return a reversed copy; the input list must not be modified.

    JAVA:   Collections.reverse(list) mutates the list you pass in, so you
            first copy: new ArrayList<>(list).
    PYTHON: lst[::-1] — a negative stride walks backwards and, like every
            slice, returns a NEW list. (lst.reverse() is the mutating twin
            and returns None.)
    """
    raise NotImplementedError


def swap_ends(lst: list) -> list:
    """
    Return a copy with the first and last elements swapped.
    Lists shorter than 2 elements are returned as an (unmodified) copy.

    JAVA:   T tmp = a[0]; a[0] = a[n-1]; a[n-1] = tmp;
    PYTHON: tuple unpacking, no temp variable:
            out[0], out[-1] = out[-1], out[0]
            (Make the copy first — e.g. list(lst) — so the input is untouched.)
    """
    raise NotImplementedError


def interleave(a: list, b: list) -> list:
    """
    Return [a[0], b[0], a[1], b[1], ...], stopping at the shorter input.

    JAVA:   index loop bounded by Math.min(a.size(), b.size()), adding one
            element from each list per iteration.
    PYTHON: zip(a, b) pairs elements and stops at the shorter input for
            free; flatten the pairs with a nested comprehension:
            [x for pair in zip(a, b) for x in pair]
    """
    raise NotImplementedError


def sort_by_length_then_alpha(words: list[str]) -> list[str]:
    """
    Return words sorted by length, ties broken alphabetically. Non-mutating.

    JAVA:   words.sort(Comparator.comparing(String::length)
                                 .thenComparing(Comparator.naturalOrder()));
    PYTHON: sorted(words, key=lambda w: (len(w), w)) — the key function maps
            each element to a tuple, and tuples compare field by field, so a
            tuple key IS a comparator chain.
    """
    raise NotImplementedError


def top_k_smallest(nums: list, k: int) -> list:
    """
    Return the k smallest numbers in ascending order (k > len is fine —
    return everything, sorted).

    JAVA:   sort a copy, then subList(0, Math.min(k, size)) — or a bounded
            PriorityQueue if you're showing off.
    PYTHON: sorted(nums)[:k]. sorted() always returns a new list, and a
            slice past the end just clamps — no min() needed.
    """
    raise NotImplementedError


def chunk(lst: list, size: int) -> list[list]:
    """
    Split lst into consecutive chunks of the given size; the last chunk may
    be shorter. chunk([], size) -> [].

    JAVA:   nested loops with careful Math.min(i + size, n) bounds on
            subList, or Guava's Lists.partition.
    PYTHON: one comprehension — slices clamp at the end automatically:
            [lst[i:i + size] for i in range(0, len(lst), size)]
    """
    raise NotImplementedError


def flatten_one_level(nested: list[list]) -> list:
    """
    Flatten a list of lists into a single list (one level deep only).

    JAVA:   nested.stream().flatMap(List::stream).collect(toList());
    PYTHON: nested comprehension, loops read left to right like the
            equivalent for-loops: [x for sub in nested for x in sub]
    """
    raise NotImplementedError
