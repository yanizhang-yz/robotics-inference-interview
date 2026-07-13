# 03 — Strings and Text

After this lesson you can take any string-manipulation warm-up — reverse the
words, check a palindrome, count characters, shift a cipher, format a report
line — and write the Python idiom directly, without translating from Java in
your head. Every term is defined at first use and every output shown was
actually run. Deep dives on the underlying rules live in
[`../LEARNING_POINTS.md`](../LEARNING_POINTS.md) (especially Gotcha 3 on
truthiness and Gotcha 10 on string building); this file stands alone for
these ten drills.

## The Java you know

This is how you'd do today's tasks in Java. Keep it in view — every section
below replaces one piece of it.

```java
// Reverse the words in a sentence, collapsing extra whitespace.
String[] words = s.trim().split("\\s+");
List<String> list = new ArrayList<>(Arrays.asList(words));
Collections.reverse(list);
String reversed = String.join(" ", list);

// Count character frequencies.
Map<Character, Integer> freq = new HashMap<>();
for (char c : s.toCharArray()) {
    freq.merge(c, 1, Integer::sum);
}

// Build a string piece by piece (never + in a loop — you use StringBuilder).
StringBuilder sb = new StringBuilder();
for (String part : parts) {
    if (!part.isEmpty()) sb.append(part).append("/");
}

// Format a report line.
String line = String.format("%-10s %d/%d (%.1f%%)",
                            name, score, total, 100.0 * score / total);
```

Five habits are hiding in there: StringBuilder, `charAt`/`toCharArray`,
regex-powered `split`, a counting HashMap, and `String.format`. Python
replaces every one of them.

## The lesson

### 1. Strings are immutable — but the fix is not StringBuilder

**Immutable** means: once created, the object can never change. Java strings
are immutable and so are Python strings — that part transfers directly. Every
string method returns a *new* string and leaves the original untouched:

```python
t = "hello"
t.upper()        # -> "HELLO"  (a new string)
print(t)         # -> hello    (t itself unchanged)
t[0] = "H"       # TypeError: 'str' object does not support item assignment
```

In Java the consequence is "use StringBuilder in loops". In Python the
consequence is different: **collect the pieces into a list, then call
`join` once at the end**:

```python
# Java: sb.append(piece) in a loop, sb.toString() at the end.
pieces = []
for x in xs:
    pieces.append(f(x))
result = "".join(pieces)          # ONE concatenation at the end
```

The gotcha to act on: `s += piece` in a loop builds a brand-new string each
time, copying everything accumulated so far — quadratic work overall, the
same trap as `String s = s + piece` in Java. Interviewers watch for `join`
here the way they'd watch for StringBuilder in Java.

`join` is a method **on the separator**, which reads backwards at first:

```python
" ".join(["fox", "brown", "quick"])   # -> "fox brown quick"
"".join(["a", "b", "c"])              # -> "abc"   (empty separator = pure concat)
",".join([1, 2])                      # TypeError: sequence item 0: expected str instance, int found
",".join(map(str, [1, 2]))            # -> "1,2"   (convert first)
```

### 2. A string is a sequence: index it, slice it, loop over it

A **sequence** in Python is anything with a length whose elements you can
address by position — strings, lists, and tuples all share the same syntax.
There is no `charAt` and no `char` type: `s[i]` gives you a **length-1
string**, and looping a string yields its characters one by one.

```python
s = "robotics"
s[0]          # -> "r"
s[-1]         # -> "s"      negative index counts from the END; -1 is the last char
for c in s:   # c is "r", then "o", ... — each a str of length 1
    ...
list("abc")   # -> ["a", "b", "c"]   explode into characters
```

A **slice** is a substring request written `s[start:stop]`. It is
**half-open**: it includes `start`, excludes `stop` — exactly like Java's
`s.substring(start, stop)`. A third number is the **step**, the stride
between characters taken:

```python
s = "robotics"
s[2:5]     # -> "bot"       chars 2, 3, 4
s[:3]      # -> "rob"       omitted start = from the beginning
s[3:]      # -> "otics"     omitted stop  = to the end
s[::2]     # -> "rbtc"      every 2nd char
s[::-1]    # -> "scitobor"  step -1 walks backwards: THE reversal idiom
```

`s[::-1]` replaces `new StringBuilder(s).reverse().toString()`. Memorize it.

Gotcha: indexing out of range throws (`s[100]` → `IndexError: string index
out of range`), but **slicing never throws** — `s[100:200]` quietly returns
`""`. Convenient in algorithms, surprising when it hides an off-by-one.

Comparisons: `<`, `>`, `==` on strings compare **lexicographically** —
character by character, like dictionary/alphabetical order, using each
character's numeric code to break the tie. So `"abc" < "abd"` is `True`, and
`"Z" < "a"` is also `True` because the code for `"Z"` (90) is smaller than
the code for `"a"` (97) — uppercase sorts before lowercase. This replaces
`compareTo`; there is no `equals`-vs-`==` trap for content comparison, `==`
compares contents.

### 3. `split` and the magic of no arguments

`s.split(",")` works like Java's `split` (minus the regex). But `s.split()`
**with no argument** is a special mode: it splits on *runs* of any whitespace
(spaces, tabs, newlines) and drops all empty strings, including at the ends.
In Java you'd need `s.trim().split("\\s+")` to get the same result.

```python
"  the quick \t brown fox ".split()   # -> ['the', 'quick', 'brown', 'fox']
"a,,b".split(",")                     # -> ['a', '', 'b']   delimiter mode KEEPS empties
"   ".split()                         # -> []               nothing but whitespace -> empty list
```

The gotcha spelled out: `split(" ")` and `split()` are different functions in
disguise. If your parsed tokens contain mysterious `''` entries, you passed
an explicit separator when you wanted the no-argument whitespace mode.

### 4. There is no `char`: `ord` and `chr` replace the casts

Every character has a numeric **code point** — its position in the Unicode
table (**Unicode** is the universal character catalog; **ASCII** is its first
128 entries, covering English letters, digits, and punctuation). Where Java
lets you do arithmetic on `char` directly via implicit int casts, Python
makes the conversion explicit with two tiny functions:

- `ord(c)` — character → code point: `ord("a")` → `97`, `ord("A")` → `65`
- `chr(n)` — code point → character: `chr(98)` → `"b"`

```java
// Java: shift a letter with wraparound — and % can go NEGATIVE.
char shifted = (char) ('a' + ((c - 'a' + k) % 26 + 26) % 26);  // the +26 dance
```

```python
# Python: % of a positive divisor is ALWAYS non-negative. No dance.
shifted = chr(ord("a") + (ord(c) - ord("a") + k) % 26)
-3 % 26      # -> 23   in Python
# in Java/C++: -3 % 26 == -3  (the sign follows the left operand)
```

That sign difference is worth a sentence of interview narration: Java's `%`
is a *remainder* whose sign follows the dividend (the left-hand operand), so
negative shifts need the `(x % 26 + 26) % 26` correction. Python's `%` with
divisor 26 always lands in `[0, 26)`, so negative `k` just works.

### 5. Counting things: `collections.Counter`

A **frequency map** (what you build with `HashMap<Character, Integer>` +
`merge`) is such a common pattern that Python ships it as a class.
`Counter(iterable)` counts every element in one call — an **iterable** is
anything a `for` loop can walk over, and a string iterates as its characters:

```python
from collections import Counter
Counter("banana")             # -> Counter({'a': 3, 'n': 2, 'b': 1})
Counter("banana")["a"]        # -> 3
Counter("banana")["z"]        # -> 0    missing key -> 0, NOT a KeyError
Counter("banana").most_common(2)   # -> [('a', 3), ('n', 2)]
Counter("listen") == Counter("silent")   # -> True   dict equality ignores order
```

`Counter` is a subclass of `dict`, so everything you know about maps applies;
wrap it in `dict(...)` if a plain dict is required. The manual version — worth
knowing because interviewers sometimes ask you to not use `Counter` — is:

```python
d = {}
for c in s:
    d[c] = d.get(c, 0) + 1    # .get(key, default) is getOrDefault
```

### 6. f-strings: `String.format` moved inside the string

An **f-string** is a string literal prefixed with `f` where `{...}` holds a
live expression, evaluated right there. After the expression, `:` introduces
a **format spec** — a mini-language controlling width, alignment, and
precision, playing the role of Java's `%-10s` / `%.1f` specifiers:

```python
name, score, total = "Yani", 17, 20
f"{name:<10}|"        # -> 'Yani      |'   < left-aligns in a 10-char field
f"{name:>10}|"        # -> '      Yani|'   > right-aligns
f"{3.14159:.2f}"      # -> '3.14'          fixed-point, 2 decimals
f"{score/total:.1%}"  # -> '85.0%'         multiplies by 100, adds %, 1 decimal
f"{42:05d}"           # -> '00042'         zero-pad to width 5
f"{255:x}"            # -> 'ff'            hex
x = 7; f"{x=}"        # -> 'x=7'           debug form: prints name AND value
```

Two gotchas to act on: `:.1%` expects the **raw ratio** (`score / total`,
not `100 * score / total`) because the `%` spec does the ×100 itself; and a
width spec never truncates — `f"{'Bartholomew':<10}"` returns the full
11-char `'Bartholomew'`, the field just expands.

### 7. Case, character tests, and truthiness

Character-class checks are **methods on the string itself**, not static
`Character.*` helpers, and they work on single characters (length-1 strings)
and whole strings alike: `c.isalnum()` (letter or digit), `c.isalpha()`,
`c.isdigit()`, `c.isspace()`.

For case-insensitive *comparison*, use `casefold()`, not `lower()`.
**Case folding** is Unicode's aggressive normalize-for-comparison lowering;
for ASCII it equals `lower()`, but e.g. German `"straße".casefold()` →
`"strasse"` while `.lower()` leaves it as `"straße"`. Saying "casefold, the
correct lower-casing for comparisons" is a free signal in interviews.

`word.capitalize()` uppercases the first character **and lowercases all the
rest**: `"aRM".capitalize()` → `"Arm"`. If you must preserve inner case, use
`word[0].upper() + word[1:]` instead.

Finally, **truthiness**: Python lets any value stand in a boolean position.
Empty things — `""`, `[]`, `{}`, `0`, `None` — count as false ("**falsy**");
everything else counts as true. So `if p:` on a string means "if p is
non-empty", replacing Java's `!p.isEmpty()`:

```python
[p for p in ["usr", "", "bin"] if p]   # -> ['usr', 'bin']  empties filtered
```

That bracketed form is a **list comprehension** — a one-line loop that builds
a list (lesson 02 covers these); the same form without brackets inside a call,
`sep.join(p for p in parts if p)`, is a **generator expression**, which
produces the items lazily without building the intermediate list.

## Muscle memory

Type these until they come out without thinking:

```python
" ".join(s.split()[::-1])                 # reverse words, normalize whitespace
s == s[::-1]                              # palindrome check
cleaned = [c.casefold() for c in s if c.isalnum()]   # sanitize a string
Counter(s)                                # frequency map in one call
d[c] = d.get(c, 0) + 1                    # manual counting, when asked
chr(ord(c) - ord("a") + ord("A"))         # character arithmetic (here: lower -> UPPER)
f"{name:<10} {ratio:.1%}"                 # aligned field + percentage
sep.join(p for p in parts if p)           # join, skipping empties
sorted(a) == sorted(b)                    # anagram check (or Counter(a) == Counter(b))
first, *rest = s.split("_")               # head/tail split of any list
```

## The drills

Work through `starter.py`; each function's docstring restates the Java
approach and the Python idiom.

### `reverse_words`

Reverse the order of words, collapsing all extra whitespace.

```python
" ".join("  the quick   brown fox ".split()[::-1])
# -> 'fox brown quick the'
```

No-arg `split()` does the trimming and collapsing, `[::-1]` reverses the
list, `join` reassembles.
**Where you'll see it:** "Reverse Words in a String" (LeetCode 151) is a
stock warm-up, and whitespace-normalizing `split()` is the first line of
nearly every log-parsing and text-tokenizing question. In robotics/ML work
it's how you chop up telemetry lines, dataset annotation rows, and CLI
output before doing anything real with them.

### `clean_palindrome`

True if the string reads the same forwards and backwards, keeping only
letters and digits, ignoring case.

```python
cleaned = [c.casefold() for c in s if c.isalnum()]
cleaned == cleaned[::-1]
# "A man, a plan, a canal: Panama" -> True
```

**Where you'll see it:** this is "Valid Palindrome" (LeetCode 125), one of
the most common screening warm-ups. The interviewer may follow up with "now
do it without extra memory" — that's the two-pointer version (your Java
instinct), so know both and say so. The sanitize-with-a-comprehension step
is also everyday data-cleaning code in ML preprocessing.

### `char_frequencies`

Map each character to its number of occurrences.

```python
dict(Counter("banana"))
# -> {'b': 1, 'a': 3, 'n': 2}
```

**Where you'll see it:** frequency maps are the substrate of a whole
interview family — "First Unique Character in a String", "Top K Frequent
Elements" (LeetCode 347), "Group Anagrams" (LeetCode 49), and most
sliding-window substring problems (the archetype where you scan a moving
window across the string, updating counts as characters enter and leave).
In ML work it's class-label counts and vocabulary building; reaching for
`Counter` instead of hand-rolling the loop is a fluency signal.

### `caesar_shift`

Shift each letter by `k` alphabet positions with wraparound, preserving case,
passing non-letters through; `k` may be negative.

```python
chr(ord("a") + (ord("x") - ord("a") + 3) % 26)   # -> 'a'  (x + 3 wraps)
-3 % 26                                          # -> 23   (no negative-% fix needed)
```

**Where you'll see it:** cipher/shift warm-ups test whether you know
`ord`/`chr` and modular index arithmetic. The same `(i + k) % n` wraparound
is the core of "Rotate Array" and of every ring buffer (a fixed-size array
whose write index wraps back to 0 at the end) — which is exactly how robot
telemetry and sensor streams are buffered in practice.

### `snake_to_camel`

Convert `snake_case` to `camelCase`; the first word stays lowercase.

```python
first, *rest = "joint_angle_limits".split("_")   # first='joint', rest=['angle','limits']
first + "".join(w.capitalize() for w in rest)
# -> 'jointAngleLimits'
```

`first, *rest = ...` is **star-unpacking**: the first element lands in
`first`, the `*` variable swallows the remainder as a list (empty if there
is nothing left — so `"robot"` and even `""` work without special cases).
**Where you'll see it:** identifier-conversion questions are common
practical screens, and the split/transform/join shape is the general answer
to all of them. In real work it's everywhere Python's `snake_case` meets a
`camelCase` JSON API or config schema — serializers do exactly this.

### `find_all_indices`

All indices where `sub` occurs in `s`, **including overlapping matches**.

```python
"aaaa".find("aa")       # -> 0
"aaaa".find("aa", 1)    # -> 1     find(sub, start) resumes the search at start
"hello".find("xyz")     # -> -1    find returns -1; .index() raises ValueError
# loop: collect i, continue from i + 1  ->  [0, 1, 2] for ("aaaa", "aa")
```

Advancing by `i + 1` (not `i + len(sub)`) is what makes matches overlap.
**Where you'll see it:** "Find the Index of the First Occurrence in a
String" (LeetCode 28, a.k.a. strStr) is the base case; overlap awareness is
the classic follow-up and shows up in pattern-repetition problems like
"Repeated Substring Pattern". Practically, it's scanning logs for every
occurrence of a marker token.

### `longest_common_prefix`

Longest prefix shared by every string in the list.

```python
list(zip(*["flower", "flow", "flight"]))
# -> [('f','f','f'), ('l','l','l'), ('o','o','i'), ('w','w','g')]
```

`zip(*strs)` **transposes** the strings — turns rows into columns: the i-th
tuple holds the i-th character of every word, and `zip` stops at the
shortest word, so there are no bounds checks. `len(set(chars)) == 1` asks
"is this column all the same character?" (a `set` collapses duplicates).
Walk columns while that holds, then join what you collected.
**Where you'll see it:** this is "Longest Common Prefix" (LeetCode 14), and
the `zip(*...)` transpose is a reusable trick for matrix-rotation questions
and for walking parallel sequences. Real-world twin: grouping file paths or
ROS topic names by shared prefix.

### `format_report`

One line: name left-aligned in 10 chars, then `score/total`, then the
percentage with 1 decimal in parentheses.

```python
f"{'Yani':<10} {17}/{20} ({17/20:.1%})"
# -> 'Yani       17/20 (85.0%)'
```

Remember: pass the raw ratio to `:.1%` — it multiplies by 100 itself.
**Where you'll see it:** rarely a question on its own, but constantly the
*output layer* of live-coding problems ("print a summary table"), and pure
daily bread in ML: training-loop progress lines, eval reports, benchmark
tables. Being able to type a format spec without docs keeps you moving
during an interview.

### `join_nonempty`

Join only the non-empty strings with the separator.

```python
"/".join(p for p in ["usr", "", "local", "", "bin"] if p)
# -> 'usr/local/bin'
```

One generator expression: truthiness (`if p`) filters the empties, `join`
handles the no-trailing-separator bookkeeping you'd do by hand around a
StringBuilder.
**Where you'll see it:** "Simplify Path" (LeetCode 71) is exactly
split-filter-join on `/`. In real code it's building file paths, URLs, and
CSV rows from optional parts — one of the highest-frequency little idioms in
any Python codebase.

### `is_anagram`

True if the two strings contain the same characters with the same counts.

```python
Counter("listen") == Counter("silent")   # -> True,  O(n)
sorted("aab") == sorted("abb")           # -> False, O(n log n) but one line
```

**Where you'll see it:** "Valid Anagram" (LeetCode 242) verbatim, scaling to
"Group Anagrams" (LeetCode 49 — use the sorted string as the dict key) and
"Find All Anagrams in a String" (a sliding window comparing counts).
Comparing two `Counter`s is also the honest way to check "same elements,
any order" in tests — you'll write it in pytest assertions for real.

## How to practice

Run the tests against the reference solutions:

```
uv run pytest ramp_up/python/03_strings_and_text -v
```

Or write your own attempt in `starter.py` and test that instead:

```
PRACTICE=1 uv run pytest ramp_up/python/03_strings_and_text -v
```
