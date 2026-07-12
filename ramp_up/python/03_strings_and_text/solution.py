"""Reference solutions: Java String/StringBuilder habits -> Python str idioms.

The through-line: Python strings are immutable like Java's, but you rarely
need a StringBuilder. Produce the pieces (list, generator), then call
sep.join(pieces) once. Slices replace reverse loops; f-strings replace
String.format; Counter replaces the counting HashMap.
"""

from collections import Counter


def reverse_words(s: str) -> str:
    """JAVA: trim().split("\\s+") + Collections.reverse + String.join.
    PYTHON: split() collapses whitespace, [::-1] reverses, " ".join stitches.
    """
    # No-arg split() drops leading/trailing/repeated whitespace for free.
    return " ".join(s.split()[::-1])


def clean_palindrome(s: str) -> bool:
    """JAVA: two-pointer scan with Character.isLetterOrDigit/toLowerCase.
    PYTHON: comprehension to clean, then compare against the [::-1] reverse.
    """
    cleaned = [c.casefold() for c in s if c.isalnum()]
    return cleaned == cleaned[::-1]


def char_frequencies(s: str) -> dict[str, int]:
    """JAVA: HashMap + map.merge(c, 1, Integer::sum).
    PYTHON: Counter(s) counts any iterable; dict(...) returns a plain dict.
    """
    return dict(Counter(s))


def caesar_shift(s: str, k: int) -> str:
    """JAVA: charAt + int casts + StringBuilder; beware negative %.
    PYTHON: ord/chr; Python's % is always non-negative, so negative k works.
    """

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
    """JAVA: StringBuilder loop uppercasing charAt(0) of each later word.
    PYTHON: star-unpack the split, capitalize the rest, join.
    """
    first, *rest = s.split("_")
    return first + "".join(word.capitalize() for word in rest)


def find_all_indices(s: str, sub: str) -> list[int]:
    """JAVA: while loop with indexOf(sub, fromIndex).
    PYTHON: str.find(sub, start) returns -1 instead of throwing; advance by 1
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
    """JAVA: shrink strs[0] while some string fails startsWith.
    PYTHON: zip(*strs) yields tuples of aligned characters (column-wise) and
    stops at the shortest string — no bounds checks needed.
    """
    prefix = []
    for chars in zip(*strs):
        if len(set(chars)) != 1:
            break
        prefix.append(chars[0])
    return "".join(prefix)


def format_report(name: str, score: int, total: int) -> str:
    """JAVA: String.format("%-10s %d/%d (%.1f%%)", ...).
    PYTHON: f-string format specs — :<10 left-aligns, :.1% turns a raw ratio
    into a percentage with one decimal and a trailing %.
    """
    return f"{name:<10} {score}/{total} ({score / total:.1%})"


def join_nonempty(parts: list[str], sep: str) -> str:
    """JAVA: StringBuilder loop with an isEmpty() guard.
    PYTHON: join accepts any iterable; empty strings are falsy, so a
    generator with `if p` filters them inline.
    """
    return sep.join(p for p in parts if p)


def is_anagram(a: str, b: str) -> bool:
    """JAVA: sort both char[] and Arrays.equals.
    PYTHON: Counter equality — O(n) and reads as the definition of anagram.
    """
    return Counter(a) == Counter(b)
