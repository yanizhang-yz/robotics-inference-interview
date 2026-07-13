# 01 — STL containers: vector, string, unordered_map, map, unordered_set

This lesson teaches you the C++ standard library containers that replace `ArrayList`,
`String`, `HashMap`, `TreeMap`, and `HashSet` — and the parameter-passing habits that C++
interviewers judge you on from your first line of code. After it, you can implement the five
drills in `starter.cpp` without looking anything up: tokenize a string, count frequencies,
sort-and-truncate a copy, group values into a sorted map, and dedup with a set.
("STL" = Standard Template Library, the historical name for the containers-and-algorithms
part of the C++ standard library. People say "STL" the way Java people say "the Collections
framework".)

## The Java you know

The five drills, written the way you would write them today:

```java
// 1. reverse the words of a sentence
List<String> words = new ArrayList<>(Arrays.asList(text.trim().split("\\s+")));
Collections.reverse(words);                          // mutates in place
String reversed = String.join(" ", words);

// 2. character frequencies
Map<Character, Integer> freq = new HashMap<>();
for (char c : text.toCharArray()) freq.merge(c, 1, Integer::sum);

// 3. k smallest values, without touching the caller's list
List<Integer> copy = new ArrayList<>(values);        // defensive copy
Collections.sort(copy);
List<Integer> topK = copy.subList(0, Math.min(k, copy.size()));

// 4. group words by length, keys iterated in sorted order
Map<Integer, List<String>> groups = new TreeMap<>();
for (String w : words) groups.computeIfAbsent(w.length(), x -> new ArrayList<>()).add(w);

// 5. sum each distinct value once
Set<Integer> seen = new HashSet<>();
long sum = 0;
for (int v : values) if (seen.add(v)) sum += v;
```

Every one of these has a direct C++ translation. The lesson below builds each piece.

## The lesson

### 1. Everything copies by default (value semantics)

In Java, every non-primitive variable is a *reference*: a small handle pointing at an object
on the heap. Assignment and parameter passing copy the handle, never the object — two
variables, one list. C++ containers instead have **value semantics**: the variable *is* the
object, and assignment or passing copies the entire contents (a fresh, independent object —
what Java would call a deep copy of the container).

```java
// Java: one list, two names
List<Integer> a = new ArrayList<>(List.of(1, 2));
List<Integer> b = a;
b.add(3);                       // a also sees [1, 2, 3]
```

```cpp
// C++: assignment CLONES the whole container
std::vector<int> a = {1, 2};
std::vector<int> b = a;         // independent copy
b.push_back(3);                 // push_back = Java's add
// a.size() -> 2, b.size() -> 3
// a == b   -> false: == compares CONTENTS ({1,2} vs {1,2,3}), never identity
```

Two consequences you must internalize:

- **Gotcha:** `void f(std::vector<int> v)` silently clones the whole vector on every call,
  and any mutation of `v` inside `f` disappears when `f` returns — the caller's vector is
  untouched. If you meant to share the object like Java does, you must say so (next section).
- **Payoff:** `==` on containers and strings compares *contents*, like Java's `.equals()`.
  There is no accidental identity-comparison bug in C++; `a == b` on two vectors is deep.

Deep dive: `../LEARNING_POINTS.md` §1.

### 2. The three ways to take a parameter

C++ makes you choose, per parameter, between copying and sharing. A **reference** (`T&`) is
an alias for the caller's object — like Java's reference, but it can never be null and can
never be re-pointed at a different object. **`const`** in front means "read-only through this name": the compiler rejects
any mutation (this is enforced, unlike Java's `final`, which only stops reassignment of the
variable, not mutation of the object).

```cpp
void f(std::vector<int> v);        // BY VALUE: full copy; mutations invisible to caller
void f(std::vector<int>& v);       // BY REFERENCE: shared, like Java; mutations visible
void f(const std::vector<int>& v); // READ-ONLY REFERENCE: no copy, no mutation allowed
```

The defaults an interviewer expects:

- Input you only read → `const std::vector<int>&`. Writing `v.push_back(1)` on it is a
  compile error ("method is not marked const"), which is the point.
- Input you would copy anyway (e.g. to sort your own version) → take it **by value** and
  mutate the parameter. The copy you were going to make *is* the parameter. Drill
  `topKSmallest` does exactly this.
- Small built-in types (`int`, `char`, `double`, `std::size_t` — the unsigned integer type C++
  uses for container sizes and indices, covered in §7) → plain by-value, copies are free.

**Gotcha:** if your function takes `const std::string& text` and you catch yourself writing
`text += "x"`, you don't need a non-const reference — you need a local copy or a separate
result string. Read-only inputs stay read-only; build outputs in a local and `return` it
(returning a local container is cheap — the compiler moves it, no clone).

### 3. `std::string` is a mutable value — no StringBuilder

Java's `String` is immutable (its contents can never change; every "modification" builds a
new object), which is why `StringBuilder` exists. `std::string` is a mutable value type:
`+=` appends **in place**, so building a string in a loop is idiomatic, not a performance bug.

```java
// Java
StringBuilder sb = new StringBuilder();
sb.append("robot"); sb.append("ics");
String s = sb.toString();
s.equals("robotics");            // true; == would compare identity
```

```cpp
// C++
std::string s = "robot";
s += "ics";                      // in-place append — this IS the StringBuilder
// s == "robotics"  -> true (== compares contents)
// s.substr(2, 3)   -> "bot"    (start index, LENGTH — not end index!)
```

**Gotcha:** `substr(i, j)` takes a *length* as its second argument, not an end index like
Java's `substring(i, j)`. `s.substr(2, 3)` is "3 characters starting at index 2".

### 4. `std::vector<T>` = ArrayList without autoboxing

In Java you juggle two families of sequences: primitive arrays (`int[]`) and object
collections (`ArrayList<Integer>`). They don't mix because generics work by *type erasure* —
the compiler erases `ArrayList<Integer>` down to a list of `Object`, so it cannot hold a bare
`int`. Java bridges the gap by silently wrapping each `int` in an `Integer` object when you
add it (**autoboxing**) and unwrapping on read (**unboxing**). That costs a heap allocation
per element and causes the classic bug:

```java
Integer a = 1000, b = 1000;
a == b;                          // false! two distinct wrapper objects
Integer c = 100, d = 100;
c == d;                          // true — the JVM caches wrappers for -128..127 only
```

C++ templates are the opposite of erasure: the compiler generates a separate, concrete class
per element type, so `std::vector<int>` stores raw 4-byte ints packed **contiguously**
(back-to-back in one memory block, like `int[]`). No wrapper objects exist, no boxing, no
`==` trap — and the contiguous layout is why C++ dominates in robotics/ML hot loops: the CPU
prefetches neighbors for free.

```cpp
std::vector<int> v = {5, 1, 4};  // literal syntax, no List.of vs Arrays.asList split
v.push_back(2);                  // add
v.size();                        // size() — returns an UNSIGNED type, see §7
v[0];                            // get(0), NO bounds check (out of range = anything can happen)
v.at(0);                         // get(0) WITH bounds check — throws std::out_of_range
v.resize(2);                     // chop (or grow) to length 2: v is now {5, 1}
std::sort(v.begin(), v.end());   // Collections.sort — free functions in <algorithm>
```

**Gotcha:** `v[i]` out of bounds does not throw — it's *undefined behavior* (the C++ term for
"the language makes no promises: crash, garbage value, or silently passing tests today are
all allowed"). While learning, use `v.at(i)` when you're not certain the index is valid.

Note the last line: algorithms like `sort`, `reverse`, `find` are standalone functions in the
`<algorithm>` header, not methods — the Java `Collections`/`Arrays` utility classes,
generalized. You hand them a range: a pair of *iterators* (§6) like `v.begin(), v.end()`.

One mechanical difference from Java's `import`: each container needs its own `#include`
(`<vector>`, `<string>`, `<unordered_map>`, `<map>`, `<unordered_set>`, `<sstream>`,
`<algorithm>`). Both `starter.cpp` and `solution.cpp` already include everything you need.

### 5. The two maps — and the `[]` that inserts

C++ splits Java's map interface exactly the way Java splits its implementations:

| Java | C++ | Under the hood |
|---|---|---|
| `HashMap<K,V>` | `std::unordered_map<K,V>` | hash table — O(1) average, no key order |
| `TreeMap<K,V>` | `std::map<K,V>` | self-balancing binary search tree — O(log n), keys iterate sorted |

(A *hash table* stores entries in an array of slots; a hash function turns each key into a
slot number, and two keys landing in the same slot — a *hash collision* — get chained
together. Same machinery as `HashMap`. Built-in types and `std::string` hash out of the box;
you only write hash code for your own key types.)

Naming trap: the **sorted** map got the short name `std::map`. If you type `std::map` out of
Java `Map` habit, you silently get `TreeMap` performance. Default to `unordered_map` unless
you need sorted keys.

The big behavioral difference is `operator[]`:

```cpp
std::unordered_map<std::string, int> m;
m["missing"];                    // does NOT return null — INSERTS {"missing", 0} and returns it
// m.size() -> 1  (!!)
```

For a missing key, `m[k]` **inserts** a *default-constructed* value (the zero-equivalent:
`0` for numbers, `""` for strings, empty vector for vectors) and returns a reference to it.
That makes counters and grouping one-liners:

```cpp
++freq[c];                       // getOrDefault(c, 0) + 1 then put, in four characters
groups[len].push_back(w);        // computeIfAbsent(len, x -> new ArrayList<>()).add(w)
```

**Gotcha:** never use `m[k]` to *test* membership — `if (m[k] == 0)` just inserted `k` into
the map. The read-only lookups are:

```cpp
if (m.count(k)) { ... }                            // containsKey
if (auto it = m.find(k); it != m.end()) {          // get, without inserting
    use(it->second);                               // -> reaches through an iterator; more in §6
}
m.at(k);                                           // get-or-throw (std::out_of_range)
```

(The `if (init; condition)` form — a semicolon *inside* the `if` — is C++17's way to declare a
variable and test it in the same line, with the variable scoped to just that `if`; here it
declares `it` and then checks `it != m.end()`. "Not found" from `find` is signaled by returning
`m.end()`, a special one-past-the-end position — not by null. `it->second` is the value;
`it->first` the key.)

### 6. Sets, pairs, and looping over everything

`std::unordered_set` is `HashSet`. Java's `add` returns a boolean; C++'s `insert` returns a
`std::pair` — a two-field struct, the standard "return two things at once" type, with fields
`.first` and `.second`. For `insert`, `.first` is the element's position and **`.second` is
Java's boolean**: `true` if the value was newly inserted.

```cpp
std::unordered_set<int> seen;
seen.insert(42).second;          // true  — first time
seen.insert(42).second;          // false — duplicate
```

Loops: the range-based `for` is Java's for-each, with one extra decision — how the loop
variable binds:

```cpp
for (int x : v)             { ... }        // copies each element: right for int/char/double
for (const auto& s : names) { ... }        // read-only alias, no copies: DEFAULT for objects
for (auto& s : names)       { s += "!"; }  // mutable alias: writes back into the container
```

(`auto` is Java's `var`; `&` makes the loop variable an alias instead of a copy.)
**Gotcha:** `for (auto s : names) s += "!";` compiles and does *nothing visible* — each
iteration mutates a throwaway copy. If a loop's writes aren't sticking, check for the
missing `&`.

Iterating a map yields pairs, and C++17 **structured bindings** — unpacking a pair/struct
into named variables in one declaration — replace `Map.Entry`:

```java
for (Map.Entry<Integer, String> e : m.entrySet()) use(e.getKey(), e.getValue());
```

```cpp
for (const auto& [key, value] : m) use(key, value);
// over a std::map, this visits keys in ascending order — {3:"c",1:"a"} prints 1:"a" 3:"c"
```

An *iterator* is C++'s generalized pointer into a container: `*it` reads the element, `++it`
advances, and a container is spanned by the pair `begin()`..`end()`. You mostly meet
iterators as the return value of `find` (§5) and as arguments to algorithms
(`std::sort(v.begin(), v.end())`). Two extras used in the drills:

- **Reverse iterators** `rbegin()`/`rend()` walk backwards — a backwards for-each with no
  index arithmetic: `for (auto it = v.rbegin(); it != v.rend(); ++it) use(*it);`
- **Iterator invalidation:** growing a vector may relocate its whole memory block, leaving
  every outstanding iterator/reference into it *dangling* — pointing at memory the vector no
  longer owns. Java throws
  `ConcurrentModificationException`; C++ gives undefined behavior. Actionable rule: don't
  hold an iterator (or call `push_back` on the vector you're looping over) across mutations.

### 7. Numbers: `long long`, and the unsigned `size()` trap

Java fixed its integer sizes forever (`int` 32-bit, `long` 64-bit). C++ only promises
minimums, and on Windows a plain `long` is 32 bits. The portable spelling of Java's `long`
is **`long long`** (or `std::int64_t` from `<cstdint>`).

Overflow is why you care: `int` tops out at 2,147,483,647. The drill `sumOfUnique` sums
values near 2 billion — an `int` accumulator overflows (which for signed integers is
undefined behavior, not Java's quiet wraparound), so the accumulator must be `long long`.

**Gotcha:** `v.size()` returns `std::size_t`, an *unsigned* (never-negative) integer type.
Unsigned arithmetic wraps: if `v` is empty, `v.size() - 1` is `18446744073709551615`, not
`-1`, so `for (i = 0; i <= v.size() - 1; ...)` on an empty vector loops ~forever. Compare
with `<` against `size()`, or loop range-based, and take sizes as `std::size_t` (as
`topKSmallest`'s `k` parameter does).

## Muscle memory

Type these without thinking; every drill below is assembled from them.

```cpp
void f(const std::vector<int>& v);            // read-only input, no copy
++freq[c];                                    // count occurrences
groups[key].push_back(x);                     // group into map-of-vectors
if (auto it = m.find(k); it != m.end())       // lookup without inserting
if (seen.insert(x).second)                    // "first time I've seen x"
std::sort(v.begin(), v.end());                // sort ascending
for (const auto& [k, v] : m)                  // iterate a map
while (stream >> word)                        // whitespace tokenizer
long long total = 0;                          // 64-bit accumulator
```

## The drills

All five live in `starter.cpp`; `main()` asserts every edge case listed here.

### `reverseWords`

Split a sentence on any whitespace, rejoin the words in reverse order, single-spaced.

The idiom: wrap the string in a `std::istringstream` (an in-memory input stream — think
`Scanner` over a string) and pull words with `>>`, which skips runs of whitespace for free;
then walk the collected vector backwards with reverse iterators.

```cpp
std::istringstream stream("  robots   move fast  ");
std::string word;
std::vector<std::string> words;
while (stream >> word) words.push_back(word);   // {"robots", "move", "fast"}
// rbegin()/rend() walk + `result += ' '; result += *it;`  -> "fast move robots"
```

Watch the separators: append `' '` *before* each word except the first (`if
(!result.empty())`), so you never end with a trailing space.

Where you'll see it: this is literally "Reverse Words in a String" (LeetCode 151), a
perennial phone-screen; the `stream >> word` tokenizer also carries "Simplify Path" and
"Compare Version Numbers"-style parsing questions. In robotics code it's the same move you
use to parse log lines, whitespace-separated sensor records, and command strings — C++ has
no `split()`, so this stream loop *is* split.

### `charFrequencies`

Count how many times each character occurs; return `unordered_map<char, int>`.

```cpp
std::unordered_map<char, int> freq;
for (char c : text) ++freq[c];
// charFrequencies("abbccc") -> {'a':1, 'b':2, 'c':3}
```

The whole drill is §5's auto-insert working *for* you: first touch of `freq[c]` inserts 0,
then `++` bumps it. Note `for (char c : text)` — a string is a container of `char`, copied
per element (cheap).

Where you'll see it: frequency maps open a huge family of interview problems — "Valid
Anagram", "Group Anagrams", "First Unique Character in a String", "Top K Frequent Elements",
"Longest Substring Without Repeating Characters" (as the sliding-window state). Interviewers
specifically watch whether you write `++freq[c]` in one stroke or fumble with
contains-then-put. Real-world: class-label histograms over a detection stream, token counts
when building a vocabulary.

### `topKSmallest`

Return the k smallest values in ascending order without modifying the caller's vector;
`k >= size` returns everything sorted, `k == 0` returns empty.

```cpp
std::vector<int> topKSmallest(std::vector<int> values, std::size_t k) {  // BY VALUE — on purpose
    std::sort(values.begin(), values.end());
    if (k < values.size()) values.resize(k);
    return values;
}
// topKSmallest({5,1,4,2,3}, 3) -> {1,2,3}; the caller's vector is untouched
```

The lesson is the signature: you need a private copy to sort, so *let the copy be the
parameter* (§2). No `new ArrayList<>(list)` line — the copy happens at the call. Then
`resize(k)` chops the tail; guard it with `k < size()` so a big `k` doesn't *grow* the
vector with zeros.

Where you'll see it: the "top-k" archetype — "Kth Largest Element in an Array" (LeetCode
215), "K Closest Points to Origin", "Top K Frequent Elements". Sort-then-truncate is the
correct first answer; expect the follow-up "can you beat O(n log n)?" — answered with a heap
or `std::nth_element` (a partial sort that places only the k-th element, average O(n)) in a
later lesson. Robotics/ML: keeping the k best detections by confidence,
k-nearest neighbors, pruning beam-search candidates.

### `groupByLength`

Bucket words by their length into a `std::map<int, vector<string>>`; keys iterate in
ascending order, words keep input order within a bucket.

```cpp
std::map<int, std::vector<std::string>> groups;
for (const auto& w : words)
    groups[static_cast<int>(w.size())].push_back(w);
// groupByLength({"go","rust","cpp","java","c"})
//   -> {1:{"c"}, 2:{"go"}, 3:{"cpp"}, 4:{"rust","java"}}   (keys visit 1,2,3,4)
```

Three lesson pieces in one line: `const auto&` loop variable (no string copies, §6),
`operator[]` creating the empty vector on first touch (§5), and `std::map` = `TreeMap`
giving you sorted keys with zero extra work. The `static_cast<int>` converts `w.size()`
(unsigned `std::size_t`, §7) to the map's `int` key explicitly — C++'s spelling of an
intentional conversion. The code compiles without it, but stricter builds (`-Wconversion`)
flag the silent 64-bit-unsigned-to-int narrowing; writing the cast says "I meant that".

Where you'll see it: map-of-vectors grouping is the exact shape of "Group Anagrams" (key =
sorted word instead of length) and every "group X by Y" warm-up; interviewers use it to
check you know `computeIfAbsent` collapses to `operator[]` here. Real-world: bucketing
detections by class id, grouping trajectory points by frame/timestamp, and any
"index this list by field" step in a data pipeline.

### `sumOfUnique`

Sum each *distinct* value exactly once (duplicates beyond the first don't count), returning
`long long`.

```cpp
std::unordered_set<int> seen;
long long total = 0;
for (int v : values)
    if (seen.insert(v).second) total += v;
// sumOfUnique({1,2,2,3,3,3}) -> 6
// sumOfUnique({2000000000, 1500000000, 2000000000}) -> 3500000000
```

`insert(v).second` is the drill: test-and-insert in a single call (§6), no separate
`contains` check. The second example is why the accumulator is `long long`: 3,500,000,000
doesn't fit in an `int` (max ~2.1 billion), and a signed overflow wouldn't wrap like Java —
it's undefined behavior (§7).

Where you'll see it: the seen-set idiom is "Contains Duplicate", "Intersection of Two
Arrays", the visited set inside every BFS/DFS ("Number of Islands" and its whole grid-search
family), and cycle detection. In robotics it's the visited set of your A*/BFS path planner
and dedup of track/point IDs across sensor frames. The 64-bit accumulator is a favorite C++
interview trap: a sum that tests green on small inputs and goes wrong at scale — reviewers
check the accumulator type before they check your loop.

## How to practice

Implement the TODO stubs in `starter.cpp`, then run the tests against your code:

```sh
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers -v
```

Or compile and run it directly — `main()` asserts every drill and prints `ALL TESTS PASSED`:

```sh
clang++ -std=c++17 -Wall -o /tmp/stl starter.cpp && /tmp/stl
```

Without `PRACTICE=1`, pytest checks the reference `solution.cpp` instead.

For the wider map of Java-to-C++ shifts (RAII, smart pointers, move semantics, headers,
undefined behavior), see `../LEARNING_POINTS.md` — this lesson covered its §1, §5, §7, §8,
and §12 in the depth this drill set needs.
