# 01 — STL containers: vector, string, unordered_map, map, unordered_set

This lesson gives you the C++ mental model from the ground up — what the compiler does,
what a variable really is, what assignment and function calls cost — and then the five
standard containers you will use every day. Every one of them has a Python counterpart
you already know cold from the Python drill sets, and this lesson leans on that: each
container is introduced as "the C++ answer to the Python tool you'd reach for". After it,
you can implement the six drills in `starter.cpp` without looking anything up: tokenize a
string, count frequencies, sort-and-truncate a copy, group values into a sorted map,
dedup with a set, and safely append to a vector you are still reading from.

("STL" = Standard Template Library, the historical name for the containers-and-algorithms
part of the C++ standard library. When someone says "STL", they mean these containers.)

Every code snippet in this lesson was compiled with `clang++ -std=c++17` and run; every
output shown in a comment is real.

## The problem this lesson solves

You arrive here fluent in Python — the drill sets in `ramp_up/python/` are behind you.
What Python never made you think about is what your data is doing *in memory*. Where does
a list actually live? What happens when you assign it to a second variable — two lists,
or two names for one list? Python's answer, from drill set 01: two names for one list,
because a Python variable is a reference, and a garbage collector quietly cleans up
whatever nothing points at anymore.

C++ answers every one of those questions differently — and makes you answer them
explicitly, in the source. That is not a hazing ritual: it is the entire reason C++ is
fast and predictable, and the reason robots and inference engines are written in it. A
control loop that must respond in 2 milliseconds cannot afford hidden costs.

The container vocabulary, at least, transfers almost one-to-one:

| Python | C++ | taught in |
|---|---|---|
| `list` | `std::vector` | step 5 |
| `str` | `std::string` | step 6 |
| `dict` | `std::unordered_map` | step 8 |
| (no stdlib sorted dict — you'd `sorted(d)` on demand) | `std::map` | step 9 |
| `set` | `std::unordered_set` | step 10 |

The trade: you must carry a small, precise mental model of memory. This lesson builds
that model one piece at a time, then teaches the containers on top of it.

## The lesson

### 1. C++ compiles the whole program before it runs

Python reads your source as it runs: `python3 script.py` starts executing line one
immediately, and a mistake on line 40 explodes only when line 40 runs. A C++ program runs
in two separate steps instead. First the **compiler** (`clang++` here) reads *all* of
your source code and translates it into **machine code** — the raw instructions your CPU
executes directly. The result is a binary file. Then you run that binary. Your source
code is not involved at runtime at all.

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

In Python that line is unremarkable — a name has no type of its own, only the object it
currently points at does, and `x = "hello"` followed by `x = 5` is everyday code. In C++
every variable declares its type up front, and the compiler holds you to it: wrong types
don't blow up at runtime, they refuse to compile. This is the first big inversion from
Python — the same mistakes, moved earlier and made louder.

Get used to this rhythm: the compiler is a strict reviewer that reads your whole program
first. A compile error is the *cheap* kind of error — nothing has run yet, nothing is
broken. Read the first error message, fix it, recompile.

Why C++ is designed this way: with no interpreter between your code and the CPU, the
speed of your program is the speed of the machine. That is what a 50 Hz robot control
loop or a GPU inference server needs.

The compiler can only be this strict because it knows the exact type — and therefore the
exact size — of every variable before the program runs. What it does with that knowledge
is the next step.

### 2. A variable is a box of bytes

Picture memory as a long row of numbered slots, one byte each. A variable is a **box**: a
small run of those slots, holding the value itself.

```cpp
int x = 42;        // a 4-byte box containing the number 42
double d = 3.14;   // an 8-byte box containing 3.14
sizeof(x);         // -> 4    (sizeof reports a box's size in bytes)
sizeof(d);         // -> 8
```

The box holds the value directly. This is the ground-level difference from Python, where
a name is a *label* tied to an object living somewhere else — drill set 01's "variables
are references" rule. A C++ variable is not a label pointing at a 42: the 4 bytes *are*
the 42. Hold onto this picture; the whole language follows from it — starting with the
question of what `=` does when there are two boxes.

### 3. Assignment copies the contents of the box

For an `int`, this is obvious and unsurprising:

```cpp
int a = 1;
int b = a;    // copy the 4 bytes
b = 99;
// a -> 1     b's change did not touch a. Two boxes, two values.
```

Now recall the rule drill set 01 drummed into you for Python lists:

```python
a = [1, 2]
b = a            # Python: two NAMES for ONE list — an alias, not a copy
b.append(3)
a                # -> [1, 2, 3]   a saw the change; same object
```

**C++ inverts this, and the inversion is the single most important thing in this
lesson.** A `std::vector<int>` (a growable array — Python's `list`; full introduction in
step 5) is also a box, and assigning it copies *all of its contents* into a brand-new,
independent container:

```cpp
std::vector<int> a = {1, 2};
std::vector<int> b = a;         // copies BOTH elements into a new vector
b.push_back(3);                 // push_back = append; grows b only
// a.size() -> 2                a still has {1, 2}. b has {1, 2, 3}.
// a == b   -> false            == compares CONTENTS, element by element
```

Two boxes, two vectors. Changing `b` cannot touch `a`, ever. Where Python's default is
*share*, C++'s default is *copy*. This is called **value semantics**: the variable *is*
the object, and copies are real copies. Retrain the instinct now — every step below
leans on it.

Why C++ is designed this way: no hidden sharing. When you hold a vector, you know with
certainty that no other part of the program can mutate it behind your back — and every
copy is visible in the source, so every cost is visible too.

The flip side: copies are real, so copies of big things are expensive. Which brings us to
function calls.

### 4. Passing to a function: copy it, or lend it

In Python, passing a list to a function hands over one more reference to the same list —
that's why a function mutating its parameter mutates the caller's data, and why passing
costs nothing no matter the size. C++ applies its own assignment rule instead: **by
default, the parameter is a copy**. Mutating the parameter mutates the copy; the caller's
variable is untouched:

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
`&`, a **reference**: an alias for the caller's actual object, not a copy of it. In other
words, Python's share-by-default behavior is available in C++ — you just ask for it
explicitly. And because sharing an object is exactly how functions mutate things behind
your back, C++ adds something Python cannot: slap `const` on the reference and the
*compiler* enforces "you may look, but you may not touch."

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

That is the whole cost model: copy, or lend. Now the containers themselves, starting
with the one you will use for everything.

### 5. `std::vector` — the default container

`std::vector<T>` is a growable array — the C++ counterpart of Python's `list`, with one
restriction visible right in the type. The `<T>` names the element type:
`std::vector<int>`, `std::vector<std::string>` — anything, but every element must be a
`T`. Python's mixed-type `[1, "two", 3.0]` has no C++ equivalent; the compiler needs one
type to know every box's size. In exchange, the elements live **contiguously**: packed
back-to-back in one solid block of memory, no gaps. Element 7 lives exactly 7 × 4 bytes
past element 0.

```cpp
std::vector<int> v = {5, 1, 4};  // a vector holding 5, 1, 4
v.push_back(2);                  // append: {5, 1, 4, 2} (grows automatically)
v.size();                        // -> 4   len(v)
v[0];                            // -> 5   read by index, NO safety check
v.at(0);                         // -> 5   read by index WITH check: bad index throws std::out_of_range
std::sort(v.begin(), v.end());   // {1, 2, 4, 5}  sort ascending, in place
v.resize(2);                     // chop to first 2 elements: {1, 2}
```

Gotcha, and your first meeting with an important term: in Python, an out-of-range index
always raises `IndexError`. C++ splits that into a choice. `v.at(i)` checks and throws,
like Python. `v[i]` does not check at all: with a bad index it is **undefined behavior**
— C++'s phrase for "the language makes no promises: a crash, a garbage value, or silently
passing tests today are all allowed outcomes." The unchecked `[]` exists because a bounds
check in a hot loop costs time and C++ refuses to charge you for safety you didn't ask
for. While learning, use `v.at(i)` whenever you're not certain the index is valid. And
retire the negative-index reflex: there is no `v[-1]` in C++ (it's undefined behavior,
not "last element") — the last element is `v.back()`.

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

We can now store a growing sequence of anything — including characters, which C++ gives
a dedicated container of their own.

### 6. `std::string` — text as a container

`std::string` is a container of `char` with text conveniences bolted on. It follows every
rule you just learned: it's a value (assignment copies it), and `==` compares contents.
The break from Python: a Python `str` is immutable — every `+=` quietly builds a whole
new string, which is why the Python track taught you `"".join(parts)` as the string
builder. A `std::string` is **mutable in place**, so `+=` really appends:

```cpp
std::string s = "robot";
s += "ics";                      // append IN PLACE: no new string is created
// s == "robotics"  -> true     == compares the characters
// s.size()         -> 8
// s.substr(2, 3)   -> "bot"    (start index, LENGTH — not an end index!)
```

Building a string with `+=` in a loop is therefore idiomatic and fast — append writes
into the existing box; no `join` needed. The one trap worth memorizing: `substr(i, n)`
takes a *length* as its second argument, where Python's `s[2:5]` takes an end index.
`s.substr(2, 3)` means "3 characters starting at index 2".

We can build strings up. The reverse chore — taking one apart — is next.

### 7. `std::istringstream` — reading tokens out of a string

You hold one string containing several values — a log line, a command, a sentence — and
you need the pieces. In Python this is one call: `text.split()`. C++ has no built-in
split function. Its tool for the job is a **stream**.

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

We can now store words and split sentences into them. The next problem: counting them.

### 8. `std::unordered_map` — the hash table

A map stores key → value pairs: `unordered_map<std::string, int>` maps strings to ints.
This is C++'s `dict` — the same machinery you met in Python drill set 04. Under the hood
it is a **hash table**: an array of slots, where a hash function converts each key into a
slot number. Lookup, insert, and erase therefore cost O(1) on average — one hash, one
slot, done — regardless of how many entries the map holds. The price: entries live in no
meaningful order (and unlike a post-3.7 Python dict, insertion order is *not* remembered
either). Built-in types and `std::string` hash out of the box.

The single most important behavior — `[]` on a missing key does not fail. It **inserts**
that key with a zero-equivalent value (`0` for numbers, `""` for strings, an empty vector
for vectors) and returns it:

```cpp
std::unordered_map<std::string, int> m;
m["missing"];                    // key not found -> INSERTS {"missing", 0}
// m.size() -> 1  (!!)
```

Python faced the same design question and gave the opposite answer: `d[k]` on a missing
key *raises* `KeyError`, and auto-insert is something you opt into with `defaultdict`.
C++ builds the `defaultdict` behavior straight into `operator[]`. That auto-insert is a
footgun for lookups but a superpower for building. You counted characters with `Counter`
in Python drill set 04 — here the same drill is a one-liner with no import: first touch
of `freq[c]` inserts 0, then `++` bumps it:

```cpp
std::unordered_map<char, int> freq;
for (char c : text) ++freq[c];   // count every character in one stroke
```

A word about `++`, since this is its first appearance. `++x` and `x++` both add 1 to
`x`. They differ only in what the *expression itself* evaluates to (verified):

```cpp
int i = 5;
int a = ++i;   // PRE-increment:  bump first, THEN produce the value -> a == 6, i == 6
int j = 5;
int b = j++;   // POST-increment: produce the OLD value, then bump  -> b == 5, j == 6
```

Post-increment must keep a copy of the old value just to hand it back — work that is
wasted when nobody uses the expression's value, as in the counting loop above. On an
`int` the compiler optimizes the difference away; on an iterator, the copy is a real
object. So the C++ habit is simple: **write `++x` unless you specifically need the old
value.** Standalone statements `++i;` and `i++;` behave identically — the habit costs
nothing and reads as fluency.

Gotcha: never use `m[k]` to *test* membership — `if (m[k] == 0)` just inserted `k`. The
read-only lookups map cleanly onto the Python vocabulary you already have:

```cpp
if (m.count(k)) { ... }                        // Python's `k in d` (returns 0 or 1)
if (auto it = m.find(k); it != m.end()) {      // Python's d.get(k): look, never insert
    use(it->second);                           // it->first is the key, it->second the value
}
m.at(k);                                       // Python's d[k]: get-or-throw (std::out_of_range)
```

(Two things to unpack there. `find` returns an iterator: a position marker pointing at
the found entry, or the special value `m.end()` — "no position" — when the key is absent.
And `if (init; condition)` — note the semicolon inside the `if` — declares a variable and
tests it in one line; the variable exists only inside that `if`.)

Why it's designed this way: O(1) lookup is bought with hashing, and hashing destroys
order. C++ gives you the fast unordered version as one container and the sorted version
as a separate one, so you only ever pay for the property you actually need.

### 9. `std::map` — the sorted tree

Same interface, different machine. Python's standard library has no sorted dict — when
drill set 04 needed keys in order, the answer was to sort on demand with `sorted(d)`.
C++ ships the sorted mapping as its own container: `std::map` stores its keys in a
**self-balancing binary search tree**, which keeps them permanently sorted. Every
operation costs O(log n) — slower than hashing — but iterating visits keys in ascending
order, guaranteed:

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

Maps hold key → value pairs. Drop the values and you get the last container on the tour.

### 10. `std::unordered_set` — membership only

A set stores keys with no values: "have I seen this before?" as a data structure —
Python's `set`, backed by the same hash table as `unordered_map`, minus the values.

`insert` does two jobs at once. It returns a `std::pair` — a two-field struct, C++'s
standard "return two things" type, fields `.first` and `.second`. For `insert`, `.second`
is the answer to "was this newly inserted?":

```cpp
std::unordered_set<int> seen;
seen.insert(42).second;          // -> true    first time
seen.insert(42).second;          // -> false   already there
```

Remember the seen-set scan from drill set 04's `first_duplicate` — `if item in seen`
then `seen.add(item)`, two hash lookups? In C++ that whole dance is one expression and
one lookup: `if (seen.insert(x).second)`. Test-and-insert in a single call — the whole
trick behind the `sumOfUnique` drill.

Five containers down. What's left is how C++ loops over them — and a choice Python never
asked you to make.

### 11. Loops: range-based `for`, `auto`, structured bindings

The range-based `for` is C++'s `for x in v`: it visits every element of a container with
no index bookkeeping. The one decision Python never asked of you is how the loop variable
binds — and it's the same copy-or-lend decision from step 4, in miniature:

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
pair into named variables right in the loop header — C++'s `for k, v in d.items()`:

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

You have met mutate-while-iterating before. Python drill set 04 showed the dict version
— `RuntimeError: dictionary changed size during iteration` — and Python's *list* version
doesn't even raise: `for x in v: v.append(...)` just loops forever, chasing the growing
end (verified). Python at least keeps the behavior defined. C++ checks nothing: the same
mistake is undefined behavior, and it can *pass your tests* until the day the block
happens to move.

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

One machine-level reality remains that Python hid completely: numbers have sizes.

### 12. Numbers: `long long`, and the unsigned `size()` trap

A Python `int` grows without limit — `2**200` just works, and no Python drill ever
mentioned overflow because Python has none. A C++ integer is a fixed-size box. Sizes
vary by platform; `int` is 4 bytes here, and its maximum value is 2,147,483,647 — about
2.1 billion. The guaranteed-64-bit integer is **`long long`** (a plain `long` is only 4
bytes on some platforms, so spell out both words). The drill `sumOfUnique` adds values
near 2 billion: an `int` accumulator overflows — and signed overflow is undefined
behavior, not a polite wraparound — so the accumulator must be `long long`. Interviewers
deliberately plant this trap: a sum that passes on small inputs and corrupts at scale.

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

In Python this whole drill is `" ".join(reversed(text.split()))` (verified). The C++
version makes each of those three steps explicit, and that's the point: pure practice of
step 7 (the stream tokenizer) plus step 11's reverse iterators. Collect the words with
`while (stream >> word)`, then walk the vector backwards with `rbegin()`/`rend()`,
appending into the result string.

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

This is drill set 04's `count_items` — `Counter(text)` — with the counting loop written
out. The whole drill is step 8's auto-insert working *for* you: first touch of `freq[c]`
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

You wrote this in Python drill set 01 as `sorted(nums)[:k]` — `sorted()` made the copy
for you and the slice clamped. The C++ lesson is the signature: you need a private copy
to sort, so *let the copy be the parameter* (step 4). No separate copy line anywhere in
the body — the copy happens at the call itself. Then `resize(k)` chops the tail; guard
it with `k < size()` so a big `k` doesn't *grow* the vector with zeros (unlike a Python
slice, `resize` doesn't clamp — it takes you at your word).

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

This is drill set 04's `defaultdict(list)` grouping pattern, upgraded: three lesson
pieces in one line. `const auto&` loop variable (no string copies, step 11),
`operator[]` creating the empty vector on first touch (step 8 — the built-in
`defaultdict`), and `std::map` giving you sorted keys with zero extra work (step 9),
where Python would sort them on demand. The `static_cast<int>` converts `w.size()`
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

The seen-set scan from drill set 04's `first_duplicate`, with a sum bolted on.
`insert(v).second` is the drill: test-and-insert in a single call (step 10), no separate
membership check. The second example is why the accumulator is `long long`:
3,500,000,000 doesn't fit in an `int` (max ~2.1 billion), and signed overflow is
undefined behavior (step 12). Python never asked you to think about the accumulator —
its ints can't overflow. C++ makes you pick the box before you pour into it.

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
this loop?"), and the modify-while-iterating mistake appears in every language — Python's
`RuntimeError: dictionary changed size during iteration` is the same crime with a kinder
sentence (step 11). In real code it's the frame queue you're draining while a callback
appends to it — which is why real pipelines hand that job to a proper queue (lesson 06).

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
