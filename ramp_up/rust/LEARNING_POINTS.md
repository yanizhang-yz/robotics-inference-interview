# A Working Programmer's Map to Rust

Dense reference card. Every section: a concept you already use, then what Rust does
with it. Read top to bottom once, then use as a lookup table.

---

## 1. Ownership replaces the garbage collector (THE core mental shift)

In a garbage-collected language, every object lives on the heap, any number of
references can point at it, and the GC figures out when it dies — assigning a
collection to a second variable just makes another alias to the same object. In Rust
there is **no GC**. Instead:

1. **Every value has exactly one owner** (a variable).
2. **Assignment and passing-by-value MOVE ownership** — the old variable is dead afterward.
3. When the owner goes out of scope, the value is freed. Deterministically. That's it.

```rust
// Rust: assignment MOVES. `a` is dead after the move.
let a = vec![1, 2, 3];
let b = a;                        // ownership moves a -> b
// println!("{}", a.len());       // COMPILE ERROR: value borrowed after move
println!("{}", b.len());          // fine; Vec freed when b goes out of scope
```

When you don't want to give a value away, you **borrow** it:

| You want | Rust | Mental model |
|---|---|---|
| Read-only access, keep ownership | `&T` (shared borrow) | Lending an object with a promise it won't be mutated |
| Mutable access, keep ownership | `&mut T` (exclusive borrow) | Lending an object to be mutated — but only ONE such reference may exist at a time |
| Give it away | `T` (move) | Handing the value off for good — the old name stops working |
| Actually share ownership | `Rc<T>` / `Arc<T>` | The closest thing to an ordinary GC-language reference (ref-counted) |

```rust
fn total(v: &Vec<i32>) -> i32 { v.iter().sum() }      // borrows, caller keeps v
fn push_one(v: &mut Vec<i32>) { v.push(1); }          // exclusive mutable borrow
fn consume(v: Vec<i32>) { /* v dropped here */ }      // takes ownership

let mut v = vec![1, 2];
let t = total(&v);        // lend it out, get it back
push_one(&mut v);         // lend it mutably
consume(v);               // gone. `v` unusable from here on.
```

**The borrow rules** (the compiler enforces these, this is "the borrow checker"):
- Any number of `&T` **or** exactly one `&mut T` at a time — never both.
- References must never outlive the thing they point to.

**Lifetimes** (`'a`) are just names for "how long this reference is valid." 90% of the
time they're inferred. When you see `fn longest<'a>(x: &'a str, y: &'a str) -> &'a str`,
read it as: "the returned reference lives no longer than the shorter-lived input."
It's the compiler making you prove you're not returning a dangling pointer — the bug
class garbage collection made impossible, solved at compile time instead of runtime.

**Cheap escape hatch while learning:** `.clone()` makes a deep copy and sidesteps the
move. Slower, but correct. Clone first, optimize later.

---

## 2. No null — `Option<T>` (absence lives in the type)

Most languages have `null` everywhere, sometimes with an optional-wrapper type bolted
on the side — and nothing stops a function from returning `null` anyway and crashing
you at runtime. Rust has **no null**. Absence is always explicit in the type, and the
compiler forces you to handle it.

```rust
// Rust — Option is not optional. If it can be absent, the type says so.
let u: Option<User> = repo.find_user(id);
let name = u.map(|u| u.name).unwrap_or_else(|| "anonymous".to_string());

// Or pattern match:
match repo.find_user(id) {
    Some(user) => println!("{}", user.name),
    None => println!("not found"),
}

// `if let` when you only care about one arm:
if let Some(user) = repo.find_user(id) {
    println!("{}", user.name);
}
```

Rosetta:

| Concept | Rust `Option` |
|---|---|
| Value present / absent | `Some(x)` / `None` |
| Transform if present | `opt.map(f)` |
| Chain another optional-returning step | `opt.and_then(f)` |
| Fall back to a default value | `opt.unwrap_or(d)` |
| Fall back to a lazily computed default | `opt.unwrap_or_else(f)` |
| Extract, crashing if absent | `opt.unwrap()` (panics) — a known code smell |
| Test for presence | `opt.is_some()` — usually a smell, pattern match instead |
| Turn absence into an error | `opt.ok_or(err)?` (converts to `Result`, see below) |

There is no null-pointer crash in safe Rust. The whole bug class is gone.

---

## 3. No exceptions — `Result<T, E>` and `?`

Rust has no `throw`/`try`/`catch`. Fallible functions return
`Result<T, E>` — an enum that is either `Ok(value)` or `Err(error)`. The error
contract lives **in the return type**: a function that can fail says so in its
signature, and the compiler warns if you ignore the result.

```rust
// Rust
use std::fs;
use std::io;

fn read(p: &str) -> Result<String, io::Error> {
    fs::read_to_string(p)          // already returns Result
}

// The `?` operator = "unwrap Ok, or early-return the Err to my caller".
// It is error re-propagation, but visible at every call site:
fn read_config() -> Result<Config, io::Error> {
    let text = fs::read_to_string("config.toml")?;   // <- on Err, return it
    let cleaned = text.trim().to_string();
    Ok(parse(cleaned))
}

// Handling at the top level:
match read_config() {
    Ok(cfg) => run(cfg),
    Err(e) => eprintln!("failed to load config: {e}"),
}
```

Key properties:
- No unchecked escape route — a function that can fail *says so in its type*, always.
- `?` makes propagation one character instead of try/catch boilerplate, so people
  actually do it instead of swallowing errors.
- `panic!` exists for unrecoverable bugs (index out of bounds, broken invariants).
  You don't catch panics in normal code.
- In application code, `Box<dyn Error>` or the `anyhow` crate is the catch-all
  error type; the `thiserror` crate is how you build a proper structured error
  hierarchy.

---

## 4. Traits: interfaces with upgrades (+ `#[derive]`)

Traits are interfaces — named bundles of methods a type promises to provide — with two
upgrades: you can implement them for types you don't own (no wrapper/adapter classes),
and there is no inheritance — composition only.

```rust
// Rust
trait Greet {
    fn greet(&self) -> String;
    fn loud(&self) -> String { self.greet().to_uppercase() }  // default method
}

struct Robot;

impl Greet for Robot {                       // impl block lives OUTSIDE the type
    fn greet(&self) -> String { "beep".to_string() }
}

// You can impl your trait for someone else's type — no wrapper class needed:
impl Greet for i32 {
    fn greet(&self) -> String { format!("I am {self}") }
}
```

Two ways to "accept an interface":

```rust
fn hello(g: &impl Greet) { ... }      // generics/monomorphized — resolved at compile time, zero-cost
fn hello_dyn(g: &dyn Greet) { ... }   // dynamic dispatch — a vtable lookup at runtime, like interface references in managed languages
```

**`#[derive]` generates the boilerplate methods** — built into the language, no
annotation processor, no magic:

```rust
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
struct Point { x: i32, y: i32 }
// Debug = printable representation, Clone = explicit deep copy,
// PartialEq/Eq = value equality, Hash = usable as a hash-map key
// Also: Default = zero-arg construction, PartialOrd/Ord = orderable
```

---

## 5. Enums with data + pattern matching

A Rust enum is a closed set of variants where **each variant can carry its own data** —
the "sealed hierarchy of record types" pattern from other languages, but native and
ergonomic.

```rust
// Rust: one enum, variants carry data
enum Shape {
    Circle { r: f64 },
    Rect { w: f64, h: f64 },
}

fn area(s: &Shape) -> f64 {
    match s {
        Shape::Circle { r } => std::f64::consts::PI * r * r,
        Shape::Rect { w, h } => w * h,
    }
}
```

`match` is exhaustive — add a variant and every `match` that
doesn't handle it **fails to compile**. `Option` and `Result` are just enums:
`enum Option<T> { Some(T), None }`. That's why pattern matching is everywhere.
Patterns also destructure, match ranges, bind with guards:

```rust
match speed {
    0 => "stopped",
    1..=30 => "slow",
    n if n > 120 => "too fast",
    _ => "cruising",              // `_` = default
}
```

---

## 6. `String` vs `&str` (one concept, two types)

| | `String` | `&str` |
|---|---|---|
| What it is | Owned, growable, heap buffer | Borrowed view into string data ("string slice") |
| Mental model | A mutable string buffer you own | A read-only window lent to you — not yours |
| Use it | struct fields, return values you build | function parameters (almost always) |

```rust
fn shout(s: &str) -> String {          // borrow in, owned out — the default signature
    s.to_uppercase()
}

let owned: String = String::from("hello");   // or "hello".to_string()
let literal: &str = "hello";                 // literals are &str
shout(&owned);       // &String auto-coerces to &str — just add &
shout(literal);
```

Rule of thumb: **take `&str`, store/return `String`.** If the compiler complains,
`.to_string()` converts `&str -> String` and `&` (or `.as_str()`) goes the other way.
Concatenation: `format!("{a}{b}")` is the general-purpose string builder.

---

## 7. Collections rosetta: `Vec` / `HashMap`

| Concept | Rust | Notes |
|---|---|---|
| Growable list, empty | `Vec::new()` or `vec![]` | `vec![1, 2, 3]` literal macro |
| Append | `v.push(x)` | |
| Index | `v[i]` (panics) / `v.get(i)` (returns `Option`) | `.get` is the safe one |
| Length | `v.len()` | |
| Emptiness | `v.is_empty()` | |
| For-each loop | `for x in &v` | `&v` borrows; bare `v` MOVES the vec into the loop |
| Hash map, empty | `HashMap::new()` | `use std::collections::HashMap;` |
| Insert | `m.insert(k, v)` | returns `Option<V>` (old value) |
| Lookup | `m.get(&k)` returns `Option<&V>` | no null surprise |
| Lookup with default | `m.get(&k).copied().unwrap_or(d)` | |
| Insert-if-absent | `m.entry(k).or_insert_with(f)` | the entry API — learn it, it's great |
| Key membership | `m.contains_key(&k)` | |
| Hash set | `HashSet::new()` | same story |
| Sort | `v.sort()` | in place; `sort_by_key(|x| ...)` for custom orderings |

```rust
// Word count — the entry API is the insert-if-absent idiom:
let mut counts: HashMap<&str, i32> = HashMap::new();
for w in text.split_whitespace() {
    *counts.entry(w).or_insert(0) += 1;
}
```

---

## 8. Immutable by default: `let` vs `let mut`

In most languages everything is mutable unless you add a constant keyword — and that
keyword usually locks only the *binding*, not the object behind it, so a "final" list
still accepts appends. Rust inverts the default — and locks the data too:

```rust
let v = vec![1, 2, 3];
// v.push(4);                // COMPILE ERROR: v is not mutable
let mut w = vec![1, 2, 3];
w.push(4);                   // ok — mutation is opt-in and visible at the declaration
```

`let` is an immutable binding that actually protects the contents. You'll also see
**shadowing** — re-declaring a name is idiomatic, not a bug:
`let x = "5"; let x: i32 = x.parse().unwrap();`
(`const` exists too, for compile-time constants.)

---

## 9. Iterators and combinators (lazy pipelines, zero-cost)

If you have used lazy filter/map/collect pipelines anywhere, this transfers directly.
Same shape, three notes: Rust iterators are lazy but **zero-cost** (they compile to the
same code as a hand-written loop), the pipeline starts straight from `.iter()` with no
wrapper step, and closures capture by borrow/move per the ownership rules.

```rust
// Rust
let mut names: Vec<String> = users.iter()
    .filter(|u| u.age >= 18)
    .map(|u| u.name.clone())
    .collect();                  // collect() infers the target from the type annotation
names.sort();
```

| Concept | Rust Iterators |
|---|---|
| Start a pipeline | `.iter()` (borrow) / `.into_iter()` (move) / `.iter_mut()` (mutate) |
| Transform / filter | `.map(f)` / `.filter(p)` |
| Materialize into a collection | `.collect::<Vec<_>>()` — also collects into `HashMap`, `String`, `Result`! |
| Fold to a single value | `.fold(init, op)` |
| Sum | `.sum::<i32>()` |
| Any / all match | `.any(p)` / `.all(p)` |
| First match | `.find(p)` → `Option` |
| Take / skip a prefix | `.take(n)` / `.skip(n)` |
| Sort | no lazy sort — `.collect()` then `.sort()` |
| Flatten nested | `.flat_map(f)` |
| Numeric range | `0..n` (ranges are iterators) |
| Count | `.count()` |
| Lockstep over two sequences | `a.iter().zip(b.iter())` |
| Index + element | `.enumerate()` |

Closures: `|x| x + 1` is the anonymous-function syntax. Multi-line: `|x| { ...; result }`.

---

## 10. Cargo: one build tool for everything

One tool, no plugins needed, no XML, lockfile by default.

| Task | Cargo |
|---|---|
| Build manifest | `Cargo.toml` (TOML, ~10 lines) |
| Package registry | crates.io |
| Compile | `cargo build` (`--release` for optimized) |
| Run tests | `cargo test` — test framework is built in, `#[test]` fn anywhere |
| Run the app | `cargo run` |
| Lint | `cargo clippy` (genuinely good — run it always) |
| Format | `cargo fmt` (one canonical style, zero config, no debates) |
| Dependency tree | `cargo tree` |
| Generate docs | `cargo doc --open` (`///` doc comments, examples in docs are compiled+run as tests) |
| Local cache | `~/.cargo` |
| New project scaffold | `cargo new my_project` |

Adding a dependency: `cargo add serde` or one line in `Cargo.toml`:
`serde = "1"`. `Cargo.lock` pins exact dependency versions by default.

---

## 11. Fearless concurrency (one paragraph)

Everything you learned to fear about shared-memory concurrency — data races, forgotten
locks, escaped references to shared mutable state — is a **compile error** in
Rust. The same ownership rules apply across threads: to move data into a thread it must
be `Send`, to share a reference between threads it must be `Sync`, and these are traits
the compiler checks automatically (auto-derived for types made of safe parts). Shared
mutable state must be explicitly wrapped — `Arc<Mutex<T>>` (a thread-safe reference +
lock fused together) — and the API makes it impossible to touch the data without holding
the lock, and impossible to forget to release it (unlock happens when the guard goes out
of scope). There is no guessing game over which synchronization keyword protects what:
if it compiles, there is no data race. Channels (`std::sync::mpsc`) cover the
producer/consumer patterns you would otherwise build on a blocking queue.

---

## Quick-glance rosetta

| Concept | Rust |
|---|---|
| Data-carrying type | `struct` |
| Interface / contract | `trait` |
| Closed set of typed variants | `enum` with data variants |
| Implementing a contract | `impl Trait for Type` |
| Boilerplate method generation | `#[derive(...)]` |
| Absence of a value | `Option<T>` (no null exists) |
| Fallible operations | `Result<T, E>` / `match` / `?` |
| Immutable binding | `let` (the default) |
| Mutable binding | `let mut` |
| Memory reclamation | ownership + drop at end of scope |
| Lending access to a value | `&T` / `&mut T` borrow |
| Printable representation | `Debug`/`Display` traits, `format!` |
| Lazy collection pipelines | iterator combinators (zero-cost) |
| Build + dependency tool | Cargo |
| Lock-protected shared state | `Mutex<T>` (compiler-enforced) |
| Doc comments | `///` + `cargo doc` |

---

Exercises coming — this track will mirror ramp_up/python/ with starter.rs / solution.rs / cargo test.
