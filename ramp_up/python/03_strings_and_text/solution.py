"""Strings and Text — reference solutions.

The through-line: Python strings are immutable, but no builder object is
needed. Produce the pieces (list, generator), then call sep.join(pieces)
once. Slices replace reverse loops; f-strings replace separate formatting
calls; Counter replaces the hand-built frequency map.
"""

from collections import Counter


def reverse_words(s: str) -> str:
    """split() collapses whitespace, [::-1] reverses, " ".join stitches."""
    # No-arg split() drops leading/trailing/repeated whitespace for free.
    return " ".join(s.split()[::-1])


def clean_palindrome(s: str) -> bool:
    """Comprehension to clean, then compare against the [::-1] reverse."""
    cleaned = [c.casefold() for c in s if c.isalnum()]
    return cleaned == cleaned[::-1]


def char_frequencies(s: str) -> dict[str, int]:
    """Counter(s) counts any iterable; dict(...) returns a plain dict."""
    return dict(Counter(s))


def caesar_shift(s: str, k: int) -> str:
    """ord/chr; Python's % is always non-negative, so negative k works."""

    def shift(c: str) -> str:
        if "a" <= c <= "z":
            base = ord("a")
        elif "A" <= c <= "Z":
            base = ord("A")
        else:
            return c
        return chr(base + (ord(c) - base + k) % 26)

    return "".join(shift(c) for c in s)


def snake_to_camel(s: str) -> str:
    """Star-unpack the split, capitalize the rest, join."""
    first, *rest = s.split("_")
    return first + "".join(word.capitalize() for word in rest)


def find_all_indices(s: str, sub: str) -> list[int]:
    """str.find(sub, start) returns -1 instead of throwing; advance by 1
    to include overlapping matches.
    """
    if not sub:
        return []
    indices = []
    i = s.find(sub)
    while i != -1:
        indices.append(i)
        i = s.find(sub, i + 1)
    return indices


def longest_common_prefix(strs: list[str]) -> str:
    """zip(*strs) yields tuples of aligned characters (column-wise) and
    stops at the shortest string — no bounds checks needed.
    """
    prefix = []
    for chars in zip(*strs):
        if len(set(chars)) != 1:
            break
        prefix.append(chars[0])
    return "".join(prefix)


def format_report(name: str, score: int, total: int) -> str:
    """f-string format specs — :<10 left-aligns, :.1% turns a raw ratio
    into a percentage with one decimal and a trailing %.
    """
    return f"{name:<10} {score}/{total} ({score / total:.1%})"


def join_nonempty(parts: list[str], sep: str) -> str:
    """join accepts any iterable; empty strings are falsy, so a
    generator with `if p` filters them inline.
    """
    return sep.join(p for p in parts if p)


def is_anagram(a: str, b: str) -> bool:
    """Counter equality — O(n) and reads as the definition of anagram."""
    return Counter(a) == Counter(b)
