# 01 — STL containers: vector, string, unordered_map, map, unordered_set

This lesson gives you the C++ mental model from the ground up — what the compiler does,
what a variable really is, what assignment and function calls cost — and then the five
standard containers you will use every day. After it, you can implement the five drills in
`starter.cpp` without looking anything up: tokenize a string, count frequencies,
sort-and-truncate a copy, group values into a sorted map, and dedup with a set.

("STL" = Standard Template Library, the historical name for the containers-and-algorithms
part of the C++ standard library. When someone says "STL", they mean these containers.)

Every code snippet in this lesson was compiled with `clang++ -std=c++17` and run; every
output shown in a comment is real.

## The problem this lesson solves

You already know how to program: variables, loops, functions, classes. What you have
probably never had to think about is what your data is doing *in memory*. Where does a
list actually live? What happens when you assign it to a second variable — do you get two
lists, or two names for one list? What does it cost to hand a list to a function?

In most languages, the runtime answers those questions for you, invisibly, and usually by
sharing: many variables quietly point at one object, and a garbage collector cleans up
later. C++ makes every one of those answers explicit and puts you in charge. That is not
a hazing ritual — it is the entire reason C++ is fast and predictable, and the reason
robots and inference engines are written in it. A control loop that must respond in 2
milliseconds cannot afford hidden costs.

The trade: you must carry a small, precise mental model. This lesson builds that model
one piece at a time, then teaches the containers on top of it.

## The lesson

### 1. C++ compiles the whole program before it runs

A C++ program runs in two separate steps. First the **compiler** (`clang++` here) reads
all of your source code and translates it into **machine code** — the raw instructions
your CPU executes directly. The result is a binary file. Then you run that binary. Your
source code is not involved at runtime at all.

```cpp
#include <iostream>

int main() {                                 // execution always starts at main()
    std::cout << "hello from machine code\n";
    return 0;
}
```

```sh
clang++ -std=c++17 -Wall -o /tmp/hello hello.cpp   # step 1: compile (this can FAIL)
/tmp/hello                                          # step 2: run the binary
```

Because the compiler reads everything before anything runs, whole categories of mistakes
are caught at compile time — the program never starts:

```cpp
int x = "hello";
// error: cannot initialize a variable of type 'int' with an lvalue of type 'const char[6]'
```

Get used to this rhythm: the compiler is a strict reviewer that reads your whole program
first. A compile error is the *cheap* kind of error — nothing has run yet, nothing is
broken. Read the first error message, fix it, recompile.

Why C++ is designed this way: with no interpreter or virtual machine between your code
and the CPU, the speed of your program is the speed of the machine. That is what a 50 Hz
robot control loop or a GPU inference server needs.

### 2. A variable is a box of bytes

Picture memory as a long row of numbered slots, one byte each. A variable is a **box**: a
small run of those slots, holding the value itself.

```cpp
int x = 42;        // a 4-byte box containing the number 42
double d = 3.14;   // an 8-byte box containing 3.14
sizeof(x);         // -> 4    (sizeof reports a box's size in bytes)
sizeof(d);         // -> 8
```

The box holds the value directly. `x` is not a link, handle, or reference to a 42 that
lives somewhere else — the 4 bytes *are* the 42. Hold onto this picture; the whole
language follows from it.

### 3. Assignment copies the contents of the box

For an `int`, this is obvious and unsurprising:

```cpp
int a = 1;
int b = a;    // copy the 4 bytes
b = 99;
// a -> 1     b's change did not touch a. Two boxes, two values.
```

Here is the part that surprises everyone: **containers work exactly the same way**. A
`std::vector<int>` (a growable array — full introduction in step 5) is also a box, and
assigning it copies *all of its contents* into a brand-new, independent container:

```cpp
std::vector<int> a = {1, 2};
std::vector<int> b = a;         // copies BOTH elements into a new vector
b.push_back(3);                 // append 3 to b
// a.size() -> 2                a still has {1, 2}. b has {1, 2, 3}.
// a == b   -> false            == compares CONTENTS, element by element
```

Two boxes, two vectors. Changing `b` cannot touch `a`, ever. This is called **value
semantics**: the variable *is* the object, and copies are real copies.

Why C++ is designed this way: no hidden sharing. When you hold a vector, you know with
certainty that no other part of the program can mutate it behind your back — and every
copy is visible in the source, so every cost is visible too.

The flip side: copies are real, so copies of big things are expensive. Which brings us to
function calls.

### 4. Passing to a function: copy it, or lend it

When you pass a variable to a function, the same assignment rule applies: **by default,
the parameter is a copy**. Mutating the parameter mutates the copy; the caller's variable
is untouched:

```cpp
void tryToGrow(std::vector<int> v) {   // v is a COPY of the caller's vector
    v.push_back(99);                   // grows the copy...
}                                      // ...which dies here

std::vector<int> mine = {1, 2};
tryToGrow(mine);
// mine.size() -> 2                    completely unaffected
```

For a big container that copy is genuinely expensive. A vector of one million `float`s is
4 MB of data (1,000,000 × 4 bytes). Passing it by value copies all 4 MB — measured on
this Mac: about **90 microseconds per call**. That's an eternity: a robot control loop at
50 Hz has a 20-millisecond budget per tick, and one careless parameter just spent half a
percent of it copying data nobody needed copied.

The alternative: instead of copying the box, hand the function the box's *address* — 8
bytes, effectively free (about a nanosecond in the same measurement). C++ spells this
`&`, a **reference**: an alias for the caller's actual object, not a copy of it. And
because sharing an object is exactly how functions mutate things behind your back, C++
lets you slap `const` on it: "you may look, but the compiler forbids you to touch."

```cpp
void f(std::vector<int> v);         // BY VALUE: copies everything; mutations invisible to caller
void f(std::vector<int>& v);        // BY REFERENCE: alias for the caller's vector; mutations visible
void f(const std::vector<int>& v);  // CONST REFERENCE: no copy, and mutation is a compile error
```

Try to mutate through a const reference and the compiler stops you — this is verified,
not folklore:

```cpp
void f(const std::vector<int>& v) { v.push_back(1); }
// error: no matching member function for call to 'push_back'
// note: 'this' argument has type 'const std::vector<int>', but method is not marked const
```

The defaults an interviewer expects from your very first line:

- Input you only read → `const std::vector<int>&`. No copy, no mutation, no exceptions.
- Input you would copy anyway (e.g. to sort your own version) → take it **by value** and
  mutate the parameter. The copy you were going to make *is* the parameter. The drill
  `topKSmallest` does exactly this.
- Small built-in types (`int`, `char`, `double`) → plain by value. Copying 4 bytes is
  cheaper than sharing them.

One habit to build now: if a read-only input tempts you to mutate it, don't reach for a
non-const reference — build your output in a local variable and `return` it. (Returning a
local container is cheap; the compiler is required to avoid the copy.)

Where you'll use this: camera frames, point clouds, and tensors are multi-megabyte
buffers arriving 30+ times per second. `const&` is how real pipelines pass them around
without ever copying by accident.

### 5. `std::vector` — the default container

`std::vector<T>` is a growable array. The `<T>` names the element type:
`std::vector<int>`, `std::vector<std::string>` — anything. Its elements live
**contiguously**: packed back-to-back in one solid block of memory, no gaps. Element 7
lives exactly 7 × 4 bytes past element 0.

```cpp
std::vector<int> v = {5, 1, 4};  // a vector holding 5, 1, 4
v.push_back(2);                  // append: {5, 1, 4, 2} (grows automatically)
v.size();                        // -> 4
v[0];                            // -> 5   read by index, NO safety check
v.at(0);                         // -> 5   read by index WITH check: bad index throws std::out_of_range
std::sort(v.begin(), v.end());   // {1, 2, 4, 5}  sort ascending
v.resize(2);                     // chop to first 2 elements: {1, 2}
```

Gotcha, and your first meeting with an important term: `v[i]` with a bad index is
**undefined behavior** — C++'s phrase for "the language makes no promises: a crash, a
garbage value, or silently passing tests today are all allowed outcomes." The unchecked
`[]` exists because a bounds check in a hot loop costs time and C++ refuses to charge you
for safety you didn't ask for. While learning, use `v.at(i)` whenever you're not certain
the index is valid.

Note the last two lines above: algorithms like `std::sort`, `std::reverse`, `std::find`
are standalone functions from the `<algorithm>` header, not methods on the container. You
hand them a range — the pair `v.begin(), v.end()` spans the whole vector. (Those two
values are **iterators**: position markers into a container. `begin()` marks the first
element; `end()` marks one-past-the-last. They come back in step 11.)

Why it's designed this way: contiguous memory is the fastest layout a CPU can traverse —
while you process element `i`, the hardware is already prefetching `i+1` because it's
literally the next bytes. This is why `vector` is THE default C++ container, and why "why
is vector fast?" is a stock interview question. When in doubt, use `vector`.

Mechanical note: each container needs its own `#include` — `<vector>`, `<string>`,
`<map>`, `<unordered_map>`, `<unordered_set>`, `<sstream>`, `<algorithm>`. Both
`starter.cpp` and `solution.cpp` already include everything the drills need.

### 6. `std::string` — text as a container

`std::string` is a container of `char` with text conveniences bolted on. It follows every
rule you just learned: it's a value (assignment copies it), it's mutable in place, and
`==` compares contents.

```cpp
std::string s = "robot";
s += "ics";                      // append IN PLACE: no new string is created
// s == "robotics"  -> true     == compares the characters
// s.size()         -> 8
// s.substr(2, 3)   -> "bot"    (start index, LENGTH — not an end index!)
```

Building a string with `+=` in a loop is idiomatic and fast — append writes into the
existing box. The one trap worth memorizing: `substr(i, n)` takes a *length* as its
second argument. `s.substr(2, 3)` means "3 characters starting at index 2".

### 7. `std::istringstream` — reading tokens out of a string

A constant chore in real code: you hold one string containing several values — a log
line, a command, a sentence — and you need the pieces. C++ has no built-in `split()`
function. Its tool for the job is a **stream**.

A stream is an object you read from one piece at a time, front to back. Keyboard input
is a stream; a file is a stream. `std::istringstream` wraps a plain string so the same
reading machinery works on it. That reuse is the design idea: C++ defines *one* reading
interface, and any source of characters can plug into it.

Reading from a stream uses `>>`, the **extraction operator**:

```cpp
#include <sstream>

std::istringstream stream("  robots   move fast  ");
std::string word;
std::vector<std::string> words;
while (stream >> word) words.push_back(word);   // {"robots", "move", "fast"}
```

`while (stream >> word)` packs three tricks into one expression. Unpack them once and
every stream loop you will ever read becomes obvious:

1. **`stream >> word` is a read.** It skips any whitespace, then copies the next run of
   non-whitespace characters into `word`. One call, one token.
2. **The expression returns the stream itself.** That is why reads chain:
   `stream >> a >> b` reads two tokens, left to right.
3. **A stream used as a condition answers "did the last read succeed?"** It stays true
   as long as reads keep producing tokens, and turns false on the first read that finds
   nothing left.

So the loop means: *try to pull a token; if one arrived, run the body; when the pull
finds only trailing whitespace, the condition turns false and the loop ends.* Step by
step on the string above:

```cpp
stream >> word   // 1st: true,  word == "robots"
stream >> word   // 2nd: true,  word == "move"
stream >> word   // 3rd: true,  word == "fast"
stream >> word   // 4th: FALSE — only trailing spaces remain; the loop exits
```

The read attempt *is* the end-of-input test — there is no separate "any words left?"
check. That is what makes the idiom worth memorizing: tokenizing needs no counters, no
index bookkeeping, no special cases for repeated or leading whitespace.

`>>` also parses numbers: reading into an `int` or `double` converts the token to a
number and fails (condition turns false) if the text is not numeric — one mechanism for
both splitting and parsing.

### 8. `std::unordered_map` — the hash table

A map stores key → value pairs: `unordered_map<std::string, int>` maps strings to ints.
Under the hood it is a **hash table**: an array of slots, where a hash function converts
each key into a slot number. Lookup, insert, and erase therefore cost O(1) on average —
one hash, one slot, done — regardless of how many entries the map holds. The price:
entries live in no meaningful order. Built-in types and `std::string` hash out of the
box.

The single most important behavior — `[]` on a missing key does not fail. It **inserts**
that key with a zero-equivalent value (`0` for numbers, `""` for strings, an empty vector
for vectors) and returns it:

```cpp
std::unordered_map<std::string, int> m;
m["missing"];                    // key not found -> INSERTS {"missing", 0}
// m.size() -> 1  (!!)
```

That auto-insert is a footgun for lookups but a superpower for building. It makes
counting a one-liner — first touch of `freq[c]` inserts 0, then `++` bumps it:

```cpp
std::unordered_map<char, int> freq;
for (char c : text) ++freq[c];   // count every character in one stroke
```

Gotcha: never use `m[k]` to *test* membership — `if (m[k] == 0)` just inserted `k`. The
read-only lookups are:

```cpp
if (m.count(k)) { ... }                        // does the key exist? (0 or 1)
if (auto it = m.find(k); it != m.end()) {      // find without inserting
    use(it->second);                           // it->first is the key, it->second the value
}
m.at(k);                                       // get-or-throw (std::out_of_range)
```

(Two things to unpack there. `find` returns an iterator: a position marker pointing at
the found entry, or the special value `m.end()` — "no position" — when the key is absent.
And `if (init; condition)` — note the semicolon inside the `if` — declares a variable and
tests it in one line; the variable exists only inside that `if`.)

Why it's designed this way: O(1) lookup is bought with hashing, and hashing destroys
order. C++ gives you the fast unordered version as one container and the sorted version
as a separate one, so you only ever pay for the property you actually need.

### 9. `std::map` — the sorted tree

Same interface, different machine. `std::map` stores its keys in a **self-balancing
binary search tree**, which keeps them permanently sorted. Every operation costs O(log n)
— slower than hashing — but iterating visits keys in ascending order, guaranteed:

```cpp
std::map<int, std::string> m;
m[3] = "c";
m[1] = "a";
// iterating m visits 1:"a" then 3:"c" — sorted, regardless of insertion order
```

Naming trap: the *sorted* container got the short name `std::map`. If you type
`std::map` on reflex you silently pay O(log n) on every operation for an ordering you may
not need. Default to `unordered_map`; reach for `map` only when you need sorted keys —
as the `groupByLength` drill does.

### 10. `std::unordered_set` — membership only

A set stores keys with no values: "have I seen this before?" as a data structure. It's
the same hash table as `unordered_map`, minus the values.

`insert` does two jobs at once. It returns a `std::pair` — a two-field struct, C++'s
standard "return two things" type, fields `.first` and `.second`. For `insert`, `.second`
is the answer to "was this newly inserted?":

```cpp
std::unordered_set<int> seen;
seen.insert(42).second;          // -> true    first time
seen.insert(42).second;          // -> false   already there
```

That one expression — `if (seen.insert(x).second)` — is test-and-insert in a single
hash lookup, and it's the whole trick behind the `sumOfUnique` drill.

### 11. Loops: range-based `for`, `auto`, structured bindings

The range-based `for` visits every element of a container without index bookkeeping. The
one decision you must make is how the loop variable binds — and it's the same
copy-or-lend decision from step 4, in miniature:

```cpp
for (int x : v)             { ... }        // copies each element: right for int/char/double
for (const auto& s : names) { ... }        // read-only alias, no copies: the DEFAULT for objects
for (auto& s : names)       { s += "!"; }  // mutable alias: writes back into the container
```

(`auto` asks the compiler to deduce the type from context — the type still exists and is
still checked; you just don't spell it. `&` makes the loop variable an alias instead of a
copy, exactly like the reference parameters of step 4.)

Gotcha (verified): `for (auto s : names) s += "!";` compiles cleanly and changes
*nothing* — each iteration mutates a throwaway copy that dies at the loop's brace. If a
loop's writes aren't sticking, look for the missing `&`.

Iterating a map yields key/value pairs, and C++17 **structured bindings** unpack each
pair into named variables right in the loop header:

```cpp
for (const auto& [key, value] : m) use(key, value);
// over a std::map this visits keys in ascending order (step 9)
```

Two iterator extras the drills use.

**Reverse iterators.** `rbegin()`/`rend()` span the container backwards, so the loop
visits the last element first, with no index arithmetic. `*it` reads the element at the
iterator's position; `++it` advances (toward the front, for a reverse iterator):

```cpp
std::vector<std::string> words = {"fast", "move", "robots"};
std::string out;
for (auto it = words.rbegin(); it != words.rend(); ++it) {
    if (!out.empty()) out += ' ';
    out += *it;
}
// out == "robots move fast"
```

**Iterator invalidation.** Growing a vector may move its entire block to a new address:
the elements must stay contiguous, and when the current block has no room to grow, the
vector allocates a bigger block elsewhere and copies everything over. You can watch it
happen — `data()` returns the block's address:

```cpp
std::vector<int> v = {1, 2, 3};
const int* before = v.data();          // remember where the block lives
for (int i = 0; i < 1000; ++i) v.push_back(i);
before == v.data()                     // -> false on this run: the block MOVED
```

Any iterator (or pointer, or reference) you saved before the growth still points at the
*old* address — freed memory. Using it is undefined behavior: it may appear to work, may
read garbage, may crash — and which one you get can change between runs. That is why
this innocent-looking loop is broken:

```cpp
for (int x : v) {          // range-for holds an iterator into v...
    v.push_back(x * 2);    // ...and push_back may relocate the block mid-loop. UB.
}
```

The safe version of "append while reading" loops by **index over the original size** —
indexes name positions, not addresses, so they survive relocation:

```cpp
std::size_t original = v.size();       // freeze the size BEFORE appending
for (std::size_t i = 0; i < original; ++i) {
    v.push_back(v[i] * 2);             // safe: v[i] re-finds the element every time
}
```

Rule: don't hold an iterator across a mutation, and never `push_back` onto the vector
you are currently range-looping over. The drill `appendDoubled` below makes you write
the safe version.

### 12. Numbers: `long long`, and the unsigned `size()` trap

C++ integer sizes vary by platform; `int` is 4 bytes here, and its maximum value is
2,147,483,647 — about 2.1 billion. The guaranteed-64-bit integer is **`long long`** (a
plain `long` is only 4 bytes on some platforms, so spell out both words). The drill
`sumOfUnique` adds values near 2 billion: an `int` accumulator overflows — and signed
overflow is undefined behavior, not a polite wraparound — so the accumulator must be
`long long`. Interviewers deliberately plant this trap: a sum that passes on small inputs
and corrupts at scale.

Gotcha (verified): `v.size()` returns `std::size_t`, an *unsigned* — never negative —
integer type. Unsigned arithmetic wraps around: on an empty vector, `v.size() - 1` is not
`-1` but `18446744073709551615`. So `for (i = 0; i <= v.size() - 1; ...)` on an empty
vector tries to loop ~18 quintillion times. Compare with `<` against `size()`, or use
range-based `for`, and take size-like parameters as `std::size_t` (as `topKSmallest`'s
`k` does).

When you must cross between integer types *on purpose*, C++ wants you to say so in the
source. **`static_cast<Target>(value)`** is the explicit conversion — target type in the
angle brackets, value in parentheses:

```cpp
std::map<int, std::vector<std::string>> groups;
groups[static_cast<int>(w.size())].push_back(w);   // size_t -> int, deliberately
```

The conversion would happen *implicitly* if you omitted the cast — this compiles either
way. But squeezing a 64-bit unsigned into a 32-bit signed int is a **narrowing**
conversion (it mangles values above ~2.1 billion), and with warnings raised
(`-Wconversion`) the implicit version gets flagged (verified):

```
warning: implicit conversion loses integer precision: 'unsigned long' to 'int'
```

The `static_cast` version stays quiet under the same flags because you have signed for
the conversion: "I know the range, this is intentional." Why C++ is designed this way:
silent number conversions are a historic bug source (the wraparound above is the same
family), so lossy conversions get a loud, searchable spelling. "Static" means the
conversion is decided entirely at compile time — no runtime cost. Prefer it over the
blunter C-style `(int)x`, which can silently perform far more dangerous conversions;
named casts are the idiom interviewers expect. (Cleanest of all is often avoiding the
mismatch: key the map by `std::size_t` and no cast is needed — a design point worth
saying out loud in an interview.)

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

Pure practice of step 7 (the stream tokenizer) plus step 11's reverse iterators:
collect the words with `while (stream >> word)`, then walk the vector backwards with
`rbegin()`/`rend()`, appending into the result string.

```cpp
// tokenize (step 7) -> {"robots", "move", "fast"}
// reverse walk (step 11) + `result += ' '; result += *it;` -> "fast move robots"
```

Watch the separators: append `' '` *before* each word except the first (`if
(!result.empty())`), so you never end with a trailing space.

Where you'll see it: this is literally "Reverse Words in a String" (LeetCode 151), a
perennial phone-screen; the `stream >> word` tokenizer also carries "Simplify Path" and
"Compare Version Numbers"-style parsing questions. In robotics code it's the same move
you use to parse log lines, whitespace-separated sensor records, and command strings —
C++ has no built-in split function, so this stream loop *is* split.

### `charFrequencies`

Count how many times each character occurs; return `unordered_map<char, int>`.

```cpp
std::unordered_map<char, int> freq;
for (char c : text) ++freq[c];
// charFrequencies("abbccc") -> {'a':1, 'b':2, 'c':3}
```

The whole drill is step 8's auto-insert working *for* you: first touch of `freq[c]`
inserts 0, then `++` bumps it. Note `for (char c : text)` — a string is a container of
`char`, copied per element (cheap).

Where you'll see it: frequency maps open a huge family of interview problems — "Valid
Anagram", "Group Anagrams", "First Unique Character in a String", "Top K Frequent
Elements", "Longest Substring Without Repeating Characters" (as the sliding-window
state). Interviewers specifically watch whether you write `++freq[c]` in one stroke or
fumble with a lookup-then-store dance. Real-world: class-label histograms over a
detection stream, token counts when building a vocabulary.

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
parameter* (step 4). No separate copy line anywhere in the body — the copy happens at the
call itself. Then `resize(k)` chops the tail; guard it with `k < size()` so a big `k`
doesn't *grow* the vector with zeros.

Where you'll see it: the "top-k" archetype — "Kth Largest Element in an Array" (LeetCode
215), "K Closest Points to Origin", "Top K Frequent Elements". Sort-then-truncate is the
correct first answer; expect the follow-up "can you beat O(n log n)?" — answered with a
heap or `std::nth_element` (a partial sort that places only the k-th element, average
O(n)) in a later lesson. Robotics/ML: keeping the k best detections by confidence,
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

Three lesson pieces in one line: `const auto&` loop variable (no string copies, step 11),
`operator[]` creating the empty vector on first touch (step 8), and `std::map` giving you
sorted keys with zero extra work (step 9). The `static_cast<int>` converts `w.size()`
(unsigned `std::size_t`, step 12) to the map's `int` key explicitly — C++'s spelling of
an intentional conversion. The code compiles without it, but stricter builds
(`-Wconversion`) flag the silent 64-bit-unsigned-to-int narrowing; writing the cast says
"I meant that".

Where you'll see it: map-of-vectors grouping is the exact shape of "Group Anagrams" (key
= sorted word instead of length) and every "group X by Y" warm-up; interviewers use it to
check whether you know `operator[]` handles the create-bucket-if-missing step by itself.
Real-world: bucketing detections by class id, grouping trajectory points by
frame/timestamp, and any "index this list by field" step in a data pipeline.

### `sumOfUnique`

Sum each *distinct* value exactly once (duplicates beyond the first don't count),
returning `long long`.

```cpp
std::unordered_set<int> seen;
long long total = 0;
for (int v : values)
    if (seen.insert(v).second) total += v;
// sumOfUnique({1,2,2,3,3,3}) -> 6
// sumOfUnique({2000000000, 1500000000, 2000000000}) -> 3500000000
```

`insert(v).second` is the drill: test-and-insert in a single call (step 10), no separate
membership check. The second example is why the accumulator is `long long`: 3,500,000,000
doesn't fit in an `int` (max ~2.1 billion), and signed overflow is undefined behavior
(step 12).

Where you'll see it: the seen-set idiom is "Contains Duplicate", "Intersection of Two
Arrays", the visited set inside every BFS/DFS ("Number of Islands" and its whole
grid-search family), and cycle detection. In robotics it's the visited set of your A*/BFS
path planner and dedup of track/point IDs across sensor frames. The 64-bit accumulator is
a favorite C++ interview trap: reviewers check the accumulator type before they check
your loop.

### `appendDoubled`

Append a doubled copy of each *original* element onto the same vector, in place:
`{1, 2, 3}` becomes `{1, 2, 3, 2, 4, 6}`.

```cpp
const std::size_t original = v.size();   // freeze the size BEFORE appending
for (std::size_t i = 0; i < original; ++i)
    v.push_back(v[i] * 2);
```

The obvious version — a range-for over `v` that push_backs into `v` — is the iterator
invalidation trap from step 11: growth can relocate the block mid-loop, and the loop's
hidden iterator keeps reading the old, freed address. Undefined behavior, and the
cruelest kind: it often *passes* on small inputs (no relocation needed yet) and corrupts
on big ones. The tests force a relocation with a 1000-element vector to catch exactly
that. Two facts make the index version safe: an index names a *position* rather than an
address, and freezing `original` up front means you never chase the growing end.

Where you'll see it: this trap is a standard C++ screening question ("what's wrong with
this loop?"), and the modify-while-iterating mistake appears in every language — deleting
from a list you're looping over is its sibling. In real code it's the frame queue you're
draining while a callback appends to it — which is why real pipelines hand that job to a
proper queue (lesson 06).

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

## The road ahead

Everything in this lesson scales straight up to inference. A tensor — the thing a neural
network actually computes on — is a contiguous buffer of floats plus a shape, which is to
say: a `std::vector<float>` wearing a lab coat. The reason step 5 hammered on contiguous
layout is that GPUs take it to the extreme — thousands of cores streaming through one
solid block of memory in lockstep. The reason step 4 hammered on `const&` is that those
buffers are hundreds of megabytes, and production inference code passes them around all
day without a single accidental copy. When you reach the CUDA lessons you'll recognize
every idea here, just wearing bigger numbers.

Next lesson: what happens when a variable's box has to *outlive* the function that
created it — ownership, and the most important idea in C++.
