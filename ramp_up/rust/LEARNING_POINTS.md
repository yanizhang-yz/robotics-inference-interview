# Java Developer's Map to Rust

Dense reference card. Every section: the Java thing you know, then what Rust does instead.
Read top to bottom once, then use as a lookup table.

---

## 1. Ownership replaces the garbage collector (THE core mental shift)

In Java, every object lives on the heap, any number of references can point at it, and
the GC figures out when it dies. In Rust there is **no GC**. Instead:

1. **Every value has exactly one owner** (a variable).
2. **Assignment and passing-by-value MOVE ownership** — the old variable is dead afterward.
3. When the owner goes out of scope, the value is freed. Deterministically. That's it.

```java
// Java: both refs point to the same list, GC cleans up eventually
List<Integer> a = new ArrayList<>(List.of(1, 2, 3));
List<Integer> b = a;              // a and b alias the same object
System.out.println(a.size());     // fine
```

```rust
// Rust: assignment MOVES. `a` is dead after the move.
let a = vec![1, 2, 3];
let b = a;                        // ownership moves a -> b
// println!("{}", a.len());       // COMPILE ERROR: value borrowed after move
println!("{}", b.len());          // fine; Vec freed when b goes out of scope
```

When you don't want to give a value away, you **borrow** it:

| You want | Rust | Java mental model |
|---|---|---|
| Read-only access, keep ownership | `&T` (shared borrow) | Passing an object you promise not to mutate |
| Mutable access, keep ownership | `&mut T` (exclusive borrow) | Passing an object to be mutated — but only ONE such reference may exist at a time |
| Give it away | `T` (move) | No Java equivalent — closest is "I null out my field after handing it off" |
| Actually share ownership | `Rc<T>` / `Arc<T>` | The closest thing to a normal Java reference (ref-counted) |

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
class Java's GC made impossible, solved at compile time instead of runtime.

**Cheap escape hatch while learning:** `.clone()` makes a deep copy and sidesteps the
move. Slower, but correct. Clone first, optimize later.

---

## 2. No null — `Option<T>` (Optional done right)

Java has `null` everywhere plus `Optional<T>` bolted on the side, and nothing stops a
method from returning `null` anyway. Rust has **no null**. Absence is always explicit
in the type, and the compiler forces you to handle it.

```java
// Java
Optional<User> u = repo.findUser(id);
String name = u.map(User::getName).orElse("anonymous");
// ...but repo could still return null and NPE you at runtime.
```

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

| Java `Optional` | Rust `Option` |
|---|---|
| `Optional.of(x)` / `Optional.empty()` | `Some(x)` / `None` |
| `opt.map(f)` | `opt.map(f)` |
| `opt.flatMap(f)` | `opt.and_then(f)` |
| `opt.orElse(d)` | `opt.unwrap_or(d)` |
| `opt.orElseGet(sup)` | `opt.unwrap_or_else(f)` |
| `opt.get()` (throws) | `opt.unwrap()` (panics) — same code smell |
| `opt.isPresent()` | `opt.is_some()` — usually a smell, pattern match instead |
| `opt.orElseThrow(...)` | `opt.ok_or(err)?` (converts to `Result`, see below) |

There is no NullPointerException in safe Rust. The whole bug class is gone.

---

## 3. No exceptions — `Result<T, E>` and `?` (checked exceptions, fixed)

Rust has no `throw`/`try`/`catch`. Fallible functions return
`Result<T, E>` — an enum that is either `Ok(value)` or `Err(error)`. Think **checked
exceptions where the "throws clause" is the return type**, and the compiler warns if
you ignore it.

```java
// Java
String read(Path p) throws IOException {
    return Files.readString(p);
}
// caller: try/catch or re-declare `throws`
```

```rust
// Rust
use std::fs;
use std::io;

fn read(p: &str) -> Result<String, io::Error> {
    fs::read_to_string(p)          // already returns Result
}

// The `?` operator = "unwrap Ok, or early-return the Err to my caller".
// It is Rust's `throws` re-propagation, but visible at every call site:
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

Key differences from checked exceptions:
- No unchecked escape route — a function that can fail *says so in its type*, always.
- `?` makes propagation one character instead of try/catch boilerplate, so people
  actually do it instead of swallowing.
- `panic!` exists (≈ `Error`/`RuntimeException` for unrecoverable bugs: index out of
  bounds, broken invariants). You don't catch panics in normal code.
- In application code, `Box<dyn Error>` or the `anyhow` crate ≈ `throws Exception`;
  the `thiserror` crate ≈ writing a proper exception hierarchy.

---

## 4. Traits vs interfaces (+ `derive` vs Lombok)

Traits are interfaces with two upgrades: you can implement them for types you don't
own (no wrapper/adapter classes), and there is no inheritance — composition only.

```java
// Java
interface Greet {
    String greet();
    default String loud() { return greet().toUpperCase(); }
}
class Robot implements Greet {
    public String greet() { return "beep"; }
}
```

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

// You can impl your trait for someone else's type — impossible in Java:
impl Greet for i32 {
    fn greet(&self) -> String { format!("I am {self}") }
}
```

Two ways to "accept an interface":

```rust
fn hello(g: &impl Greet) { ... }      // generics/monomorphized — like Java generics but zero-cost, resolved at compile time
fn hello_dyn(g: &dyn Greet) { ... }   // dynamic dispatch — this is what a Java interface reference actually does (vtable)
```

**`#[derive]` is Lombok built into the language** — no annotation processor, no magic:

```java
// Java + Lombok
@Data @EqualsAndHashCode @ToString
class Point { int x; int y; }
```

```rust
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
struct Point { x: i32, y: i32 }
// Debug ≈ toString, Clone ≈ clone(), PartialEq/Eq ≈ equals(), Hash ≈ hashCode()
// Also: Default ≈ no-arg constructor, PartialOrd/Ord ≈ Comparable
```

---

## 5. Enums with data + pattern matching vs sealed classes/switch

Rust enums are Java 17+ sealed interfaces + records, but native and ergonomic.
Each variant can carry its own data.

```java
// Java (modern): sealed hierarchy + pattern switch
sealed interface Shape permits Circle, Rect {}
record Circle(double r) implements Shape {}
record Rect(double w, double h) implements Shape {}

double area(Shape s) {
    return switch (s) {
        case Circle c -> Math.PI * c.r() * c.r();
        case Rect r -> r.w() * r.h();
    };
}
```

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

`match` is exhaustive like a sealed-switch — add a variant and every `match` that
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

## 6. `String` vs `&str` (Java has one string type; Rust has two)

| | `String` | `&str` |
|---|---|---|
| What it is | Owned, growable, heap buffer | Borrowed view into string data ("string slice") |
| Java analogy | `StringBuilder` you own | `String` passed to a method — read-only, not yours |
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
Concatenation: `format!("{a}{b}")` is your `String.format`/`+`.

---

## 7. Collections rosetta: `Vec` / `HashMap`

| Java | Rust | Notes |
|---|---|---|
| `new ArrayList<>()` | `Vec::new()` or `vec![]` | `vec![1, 2, 3]` literal macro |
| `list.add(x)` | `v.push(x)` | |
| `list.get(i)` (throws OOB) | `v[i]` (panics) / `v.get(i)` (returns `Option`) | `.get` is the safe one |
| `list.size()` | `v.len()` | |
| `list.isEmpty()` | `v.is_empty()` | |
| `for (var x : list)` | `for x in &v` | `&v` borrows; bare `v` MOVES the vec into the loop |
| `new HashMap<>()` | `HashMap::new()` | `use std::collections::HashMap;` |
| `map.put(k, v)` | `m.insert(k, v)` | returns `Option<V>` (old value) |
| `map.get(k)` (nullable!) | `m.get(&k)` returns `Option<&V>` | no null surprise |
| `map.getOrDefault(k, d)` | `m.get(&k).copied().unwrap_or(d)` | |
| `map.computeIfAbsent(k, f)` | `m.entry(k).or_insert_with(f)` | the entry API — learn it, it's great |
| `map.containsKey(k)` | `m.contains_key(&k)` | |
| `new HashSet<>()` | `HashSet::new()` | same story |
| `Collections.sort(list)` | `v.sort()` | in place; `sort_by_key(|x| ...)` ≈ `Comparator.comparing` |

```rust
// Word count — the entry API replaces computeIfAbsent/merge:
let mut counts: HashMap<&str, i32> = HashMap::new();
for w in text.split_whitespace() {
    *counts.entry(w).or_insert(0) += 1;
}
```

---

## 8. Immutable by default: `let` vs `let mut` (vs `final`)

Java: everything is mutable unless you write `final` (and `final` only locks the
reference, not the object). Rust inverts it — and locks the data too:

```java
final List<Integer> xs = new ArrayList<>();
xs.add(1);                   // fine! final didn't protect the contents
```

```rust
let v = vec![1, 2, 3];
// v.push(4);                // COMPILE ERROR: v is not mutable
let mut w = vec![1, 2, 3];
w.push(4);                   // ok — mutation is opt-in and visible at the declaration
```

`let` ≈ `final` that actually works. You'll also see **shadowing** — re-declaring a
name is idiomatic, not a bug: `let x = "5"; let x: i32 = x.parse().unwrap();`
(`const` exists too, for compile-time constants ≈ `static final`.)

---

## 9. Iterators and combinators (your Streams knowledge transfers)

This is the most direct port of a Java skill you have. Same shape, three differences:
Rust iterators are also lazy but **zero-cost** (compile to the same code as a hand-written
loop), there's no `.stream()` prefix noise, and closures capture by borrow/move per the
ownership rules.

```java
// Java
List<String> names = users.stream()
    .filter(u -> u.age() >= 18)
    .map(User::name)
    .sorted()
    .collect(Collectors.toList());
```

```rust
// Rust
let mut names: Vec<String> = users.iter()
    .filter(|u| u.age >= 18)
    .map(|u| u.name.clone())
    .collect();                  // collect() infers the target from the type annotation
names.sort();
```

| Java Streams | Rust Iterators |
|---|---|
| `.stream()` | `.iter()` (borrow) / `.into_iter()` (move) / `.iter_mut()` (mutate) |
| `.map(f)` / `.filter(p)` | `.map(f)` / `.filter(p)` |
| `.collect(toList())` | `.collect::<Vec<_>>()` — also collects into `HashMap`, `String`, `Result`! |
| `.reduce(id, op)` | `.fold(init, op)` |
| `.mapToInt(...).sum()` | `.sum::<i32>()` |
| `.anyMatch(p)` / `.allMatch(p)` | `.any(p)` / `.all(p)` |
| `.findFirst()` | `.find(p)` → `Option` |
| `.limit(n)` / `.skip(n)` | `.take(n)` / `.skip(n)` |
| `.sorted()` | no lazy sort — `.collect()` then `.sort()` |
| `.flatMap(f)` | `.flat_map(f)` |
| `IntStream.range(0, n)` | `0..n` (ranges are iterators) |
| `.count()` | `.count()` |
| zip two streams (awkward) | `a.iter().zip(b.iter())` |
| index + element (awkward) | `.enumerate()` |

Closures: `|x| x + 1` ≈ `x -> x + 1`. Multi-line: `|x| { ...; result }`.

---

## 10. Cargo vs Maven/Gradle

One tool, no plugins needed, no XML, lockfile by default.

| Maven/Gradle | Cargo |
|---|---|
| `pom.xml` / `build.gradle` | `Cargo.toml` (TOML, ~10 lines) |
| Maven Central | crates.io |
| `mvn compile` | `cargo build` (`--release` for optimized) |
| `mvn test` (Surefire) | `cargo test` — test framework is built in, `#[test]` fn anywhere |
| `mvn exec:java` | `cargo run` |
| Checkstyle/SpotBugs | `cargo clippy` (linter, genuinely good — run it always) |
| google-java-format | `cargo fmt` (one canonical style, zero config, no debates) |
| `mvn dependency:tree` | `cargo tree` |
| Javadoc | `cargo doc --open` (`///` doc comments, examples in docs are compiled+run as tests) |
| `~/.m2` | `~/.cargo` |
| archetype | `cargo new my_project` |

Adding a dependency: `cargo add serde` or one line in `Cargo.toml`:
`serde = "1"`. `Cargo.lock` ≈ a lockfile Maven never gave you.

---

## 11. Fearless concurrency (one paragraph)

Everything you learned to fear about Java concurrency — data races, forgotten
`synchronized`, escaped references to shared mutable state — is a **compile error** in
Rust. The same ownership rules apply across threads: to move data into a thread it must
be `Send`, to share a reference between threads it must be `Sync`, and these are traits
the compiler checks automatically (auto-derived for types made of safe parts). Shared
mutable state must be explicitly wrapped — `Arc<Mutex<T>>` (≈ a thread-safe reference +
lock fused together) — and the API makes it impossible to touch the data without holding
the lock, and impossible to forget to release it (unlock happens when the guard goes out
of scope). There is no `volatile`-vs-`synchronized`-vs-`Atomic` guessing game: if it
compiles, there is no data race. Channels (`std::sync::mpsc`) cover the
producer/consumer patterns you'd build with `BlockingQueue`.

---

## Quick-glance rosetta

| Java | Rust |
|---|---|
| `class` (data) | `struct` |
| `interface` | `trait` |
| `sealed interface` + records | `enum` with data variants |
| `implements` | `impl Trait for Type` |
| Lombok / records | `#[derive(...)]` |
| `Optional<T>` / `null` | `Option<T>` (no null exists) |
| `throws E` / try-catch | `Result<T, E>` / `match` / `?` |
| `final` local | `let` (default) |
| mutable local | `let mut` |
| GC | ownership + drop at end of scope |
| pass reference | `&T` / `&mut T` borrow |
| `toString()` | `Debug`/`Display` traits, `format!` |
| Streams | iterator combinators (lazy, zero-cost) |
| Maven/Gradle | Cargo |
| `synchronized` | `Mutex<T>` (compiler-enforced) |
| JavaDoc | `///` + `cargo doc` |

---

Exercises coming — this track will mirror ramp_up/python/ with starter.rs / solution.rs / cargo test.
