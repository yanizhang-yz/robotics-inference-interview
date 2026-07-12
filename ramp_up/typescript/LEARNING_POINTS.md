# Java Developer's Map to TypeScript

Dense reference card. Every section: the Java thing you know, then what TypeScript does
instead. TS is "Java-shaped" on the surface — that's exactly why it bites: the syntax
looks familiar while the type system plays by different rules.

---

## 1. Structural typing vs nominal typing (THE biggest shift)

Java types are **nominal**: a class satisfies an interface only if it declares
`implements`. TypeScript types are **structural**: **if it has the shape, it fits.**
Nobody has to declare anything.

```java
// Java: MUST declare the relationship
interface Named { String getName(); }
class User implements Named {           // <- required
    public String getName() { return "yani"; }
}
```

```ts
// TypeScript: shape is all that matters
interface Named { name: string; }

const user = { name: "yani", age: 38 };   // never heard of Named
function greet(n: Named) { console.log(n.name); }
greet(user);                              // ✓ compiles — it has a `name: string`, so it IS a Named
```

Consequences that will surprise you:
- Two independently-defined interfaces with the same members are **the same type**.
  There is no "which interface did you mean" — only shapes.
- You can pass object literals anywhere the shape matches; no adapter classes, no
  wrapping, no `implements` ceremony.
- Extra properties are fine on variables passed in (`user` above has `age`), but
  object literals passed *directly* get "excess property checks":
  `greet({ name: "x", age: 1 })` errors — a lint-like safety net, not nominal typing.
- Nominal-style branding is possible but rare; embrace shapes.

---

## 2. Type inference — stop writing the types Java made you write

TypeScript infers aggressively. Writing types the compiler already knows is noise.

```java
// Java (pre-var, and even var is limited)
Map<String, List<Integer>> scores = new HashMap<String, List<Integer>>();
String name = user.getName();
```

```ts
// TypeScript — inferred, still fully type-checked
const scores = new Map<string, number[]>();  // scores: Map<string, number[]>
const name = user.name;                      // name: string
const doubled = [1, 2, 3].map(x => x * 2);   // doubled: number[] — even through lambdas
```

Where you DO write types:
- **Function parameters and public return types** — the API boundary.
- Empty containers where there's nothing to infer: `const xs: string[] = [];`
- That's roughly it. If your locals all have annotations, you're writing Java in TS.

`as const` bonus: `const dirs = ["N", "E", "S", "W"] as const;` gives the precise type
`readonly ["N", "E", "S", "W"]` instead of `string[]` — no Java equivalent.

---

## 3. Union types + narrowing vs class hierarchies + `instanceof`

Where Java reaches for a sealed interface hierarchy, TS reaches for a **union of plain
shapes** and **narrows** with ordinary control flow. The compiler tracks `if` branches
and refines the type — no casting.

```java
// Java: sealed hierarchy + pattern switch
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

```ts
// TypeScript: discriminated union — the `kind` field is the discriminator
type Circle = { kind: "circle"; r: number };
type Rect = { kind: "rect"; w: number; h: number };
type Shape = Circle | Rect;

function area(s: Shape): number {
  switch (s.kind) {
    case "circle": return Math.PI * s.r * s.r;  // s narrowed to Circle here
    case "rect":   return s.w * s.h;            // s narrowed to Rect here
  }
}
```

Narrowing tools (the compiler understands all of these):

| Check | Narrows by |
|---|---|
| `typeof x === "string"` | primitive type |
| `x instanceof Date` | class instances (classes only) |
| `"w" in s` | property presence |
| `s.kind === "rect"` | discriminant literal — the workhorse |
| `if (x != null)` / `if (x)` | strips `null`/`undefined` |

Exhaustiveness ≈ sealed-switch: add a `default: const _: never = s;` arm — if a new
variant appears, that line stops compiling. Unions also do what Java can't:
`type Id = string | number;` or `type Direction = "N" | "E" | "S" | "W";`
(string literal unions replace most enums).

---

## 4. Null handling: `strictNullChecks`, `?.`, `??` vs `Optional`

With `strictNullChecks: true` (non-negotiable — see tsconfig section), `null` and
`undefined` are **not** members of other types. `string` cannot hold null. Absence must
be declared, and the compiler forces a check — this is `Optional` built into the type
system instead of a wrapper class.

```java
// Java
Optional<User> u = findUser(id);
String city = u.map(User::getAddress)
               .map(Address::getCity)
               .orElse("unknown");
```

```ts
// TypeScript — `| undefined` in the type IS the Optional
function findUser(id: string): User | undefined { ... }

const u = findUser(id);
// u.name;                          // COMPILE ERROR: u is possibly undefined
const city = u?.address?.city ?? "unknown";   // one line, no wrapper type
```

- `?.` — optional chaining ≈ chained `Optional.map`; short-circuits to `undefined`.
- `??` — nullish coalescing ≈ `orElse`. Use `??`, not `||`: `0 || "x"` gives `"x"`
  (0 is falsy!), `0 ?? "x"` gives `0`. `||` on numbers/strings is a classic bug.
- `x!` — non-null assertion ≈ `Optional.get()`: "trust me." Same smell, same NPE risk.
- Optional properties: `interface Config { retries?: number }` — the `?` means
  `number | undefined`.
- Two nulls exist: `undefined` (absent — the default, use this) and `null`
  (explicit empty — mostly from JSON/APIs). `??` and `?.` treat both the same;
  `x == null` (double equals, deliberately) catches both.

Nobody uses an `Optional<T>` class in TS. The union type + `?.`/`??` is the idiom.

---

## 5. `interface` vs `type` alias vs `class` — and why classes are rare

Three keywords where Java has one. The surprise: **`class` is the one you'll use least.**

| | Use for | Java analogy |
|---|---|---|
| `interface` | Object shapes, especially public APIs; can be extended/merged | `interface`, but purely structural |
| `type` | Unions, intersections, aliases of anything: `type Id = string \| number` | no equivalent — closest is a sealed marker hierarchy |
| `class` | When you need instances with behavior + `instanceof` + private state | `class` — but see below |

```ts
interface Pose { x: number; y: number; theta: number; }     // shape
type Meters = number;                                        // alias
type Command = MoveCmd | StopCmd | HomeCmd;                  // union — needs `type`

// What Java would make a class, TS usually makes a plain object + functions:
const start: Pose = { x: 0, y: 0, theta: 0 };
function advance(p: Pose, d: Meters): Pose {
  return { ...p, x: p.x + d * Math.cos(p.theta), y: p.y + d * Math.sin(p.theta) };
}
```

Why classes are rare: structural typing means you don't need a class to satisfy an
interface; object literals are cheap and typed; functions are first-class so you don't
need `FooStrategy` classes (just pass a function); and `{ ...p, x: 1 }` spread gives
you immutable-update "withers" for free. Reach for `class` when you genuinely want
encapsulated mutable state + methods (a connection pool, a simulator) — not for DTOs,
not for services that are really just namespaced functions.

`interface` vs `type` in practice: for plain object shapes they're interchangeable;
default to `interface` for public object shapes, `type` for unions and everything else.

---

## 6. `async`/`await` + Promises vs `CompletableFuture`/threads

`Promise<T>` ≈ `CompletableFuture<T>`, and `async/await` is the syntax Java never gave
you for it. The deeper difference: **JavaScript is single-threaded.** One event loop.
No threads, no `synchronized`, no `volatile`, no locks — and no data races, because two
pieces of your code never run simultaneously. Concurrency = interleaving awaits, not
parallelism.

```java
// Java
CompletableFuture<User> fut = fetchUser(id)
    .thenCompose(u -> fetchOrders(u))
    .thenApply(orders -> summarize(orders))
    .exceptionally(e -> fallback());
```

```ts
// TypeScript — reads like blocking code, runs non-blocking
async function loadSummary(id: string): Promise<Summary> {
  try {
    const user = await fetchUser(id);       // yields the event loop; nothing is blocked
    const orders = await fetchOrders(user);
    return summarize(orders);
  } catch (e) {
    return fallback();                       // try/catch works across awaits
  }
}
```

Rosetta:

| Java | TypeScript |
|---|---|
| `CompletableFuture<T>` | `Promise<T>` |
| `.thenApply(f)` / `.thenCompose(f)` | `await` (or `.then(f)` — prefer await) |
| `.exceptionally(f)` | `try { await ... } catch` |
| `CompletableFuture.allOf(a, b)` | `await Promise.all([a, b])` — typed tuple result! |
| `anyOf` | `Promise.race` / `Promise.any` |
| `ExecutorService`, thread pools | none — the event loop schedules everything |
| `synchronized`, locks | not needed; no shared-memory parallelism |
| blocking `.get()`/`.join()` | doesn't exist — you can't block the loop, only await |

Gotchas: an `async` function always returns a `Promise` (even `async fn(): Promise<void>`);
forgetting `await` gives you a `Promise<T>` where you wanted `T` — the type checker
catches this, read the error; a forgotten await that's *discarded* is a floating promise
(lint rule: `no-floating-promises`); `await` in a plain `for` loop is sequential — use
`Promise.all(items.map(...))` for "parallel" fan-out. CPU-bound work still blocks
everything — there's no thread to hide it on (that's what worker threads are for, rarely).

---

## 7. Generics: look like Java, erase differently

The syntax transfers (`<T>`, `<T extends Base>`), and both erase at runtime — but TS
generics are structural and inferred, and the *compile-time* system is far more capable.

```java
// Java
<T extends Comparable<T>> T max(List<T> xs) { ... }
```

```ts
// TypeScript
function max<T>(xs: T[], cmp: (a: T, b: T) => number): T { ... }
const m = max([3, 1, 2], (a, b) => a - b);   // T inferred as number — no <> at call site
```

Differences that matter:
- **Inference at call sites**: you almost never write `max<number>(...)`.
- **No wrapper types**: `number[]` just works; no `List<Integer>` boxing distinction.
- **Erasure is total**: like Java you can't do `x instanceof T`, and there's no
  `Class<T>` token workaround either — narrow with runtime checks on structure instead.
- **`keyof` and friends** (no Java equivalent): `function get<T, K extends keyof T>(obj: T, key: K): T[K]`
  — a getter that's type-safe per property name. Also `Partial<T>` (all fields
  optional), `Pick<T, K>`, `Record<K, V>` — types computed *from other types*.
- **Variance is inferred** from usage, not declared: no `? extends` / `? super`
  wildcards to memorize. (Arrays/functions behave mostly as you'd hope; you'll rarely
  think about it.)
- Default type params exist: `function make<T = string>(): T[] { return []; }`.

---

## 8. `const`/`let` and scoping — no hoisting surprises (if you do it right)

| Keyword | Meaning | Java analogy |
|---|---|---|
| `const` | Block-scoped, no reassignment — **your default** | `final` local (same caveat: object contents still mutable) |
| `let` | Block-scoped, reassignable | normal local variable |
| `var` | Function-scoped, hoisted, legacy — **never use** | no analogy; a footgun |

```ts
const xs = [1, 2, 3];
xs.push(4);            // fine — const locks the binding, not the array (exactly like final)
// xs = [];            // error

for (let i = 0; i < 3; i++) { setTimeout(() => console.log(i)); }  // 0 1 2 — let is per-iteration
// with `var i`, this prints 3 3 3 — the classic JS interview gotcha, avoided by never using var
```

Stick to `const` (reach for `let` only when you actually reassign, e.g. accumulators)
and scoping behaves exactly like Java blocks. Deep immutability when you want it:
`readonly` properties, `ReadonlyArray<T>`, `as const`.

---

## 9. Array methods vs Streams: eager, chainable, everywhere

Your Streams fluency ports directly — minus the `.stream()`/`.collect()` bookends.
Arrays *are* the pipeline. One real difference: these are **eager** (each step builds a
new array immediately, no lazy fusion). For interview-sized data, irrelevant.

```java
// Java
List<String> names = users.stream()
    .filter(u -> u.age() >= 18)
    .map(User::name)
    .sorted()
    .collect(Collectors.toList());
```

```ts
// TypeScript — no bookends
const names = users
  .filter(u => u.age >= 18)
  .map(u => u.name)
  .toSorted();          // non-mutating sort (ES2023); .sort() sorts IN PLACE — gotcha
```

| Java Streams | TS arrays |
|---|---|
| `.stream()` / `.collect(...)` | not needed |
| `.map(f)` / `.filter(p)` | `.map(f)` / `.filter(p)` |
| `.reduce(id, op)` | `.reduce(op, init)` — **args swapped: init comes second** |
| `.anyMatch` / `.allMatch` | `.some(p)` / `.every(p)` |
| `.findFirst().orElse(null)` | `.find(p)` → `T \| undefined` |
| `.flatMap(f)` | `.flatMap(f)` |
| `.sorted(cmp)` | `.toSorted(cmp)` — comparator returns a number, `(a,b) => a - b` |
| `.limit(n)` / `.skip(n)` | `.slice(0, n)` / `.slice(n)` |
| `.count()` | `.filter(p).length` |
| `.forEach` | `.forEach` (or `for...of` — use `of`, never `in`) |
| `Collectors.groupingBy` | `Object.groupBy(xs, f)` (ES2024) or a reduce into a `Map` |
| `IntStream.range(0, n)` | `Array.from({ length: n }, (_, i) => i)` (or just a for loop) |
| `.distinct()` | `[...new Set(xs)]` |

Mutation traps from Java-land: `.sort()`, `.reverse()`, `.splice()` mutate in place
(use `.toSorted()`, `.toReversed()`, `.slice()`); `.map` on a `Map` doesn't exist —
`Map`/`Set` iterate via `for...of` or `[...map.entries()]` first.

---

## 10. npm + tsconfig vs Maven

| Maven/Gradle | Node/TS |
|---|---|
| `pom.xml` | `package.json` (deps + scripts) |
| Maven Central | npm registry |
| `mvn install` | `npm install` |
| `pom.xml` `<dependencies>` | `"dependencies"` / `"devDependencies"` (dev ≈ test+provided scope) |
| version locking (nope) | `package-lock.json` — commit it |
| `mvn test` | `npm test` → runs whatever `"scripts": { "test": ... }` says (vitest here) |
| `mvn exec` | `npm run <script>` — scripts are the build lifecycle, freeform |
| `javac` | `tsc` — but often only type-CHECKS (`tsc --noEmit`); a bundler or `tsx` actually runs it |
| JDK version | `"target"`/`"lib"` in tsconfig + Node version |
| compiler flags | `tsconfig.json` — the important file |
| `~/.m2` | `node_modules/` per project (yes, it's huge; yes, that's normal) |

The `tsconfig.json` block that makes TS worth using — always start from:

```jsonc
{
  "compilerOptions": {
    "strict": true,            // the whole ballgame: strictNullChecks, noImplicitAny, etc.
    "target": "ES2022",
    "module": "ESNext",
    "moduleResolution": "bundler",
    "noUncheckedIndexedAccess": true   // arr[i] is T | undefined — honest about bounds
  }
}
```

Without `"strict": true`, TypeScript silently degrades toward JavaScript with
decorative annotations. Non-negotiable.

---

## 11. The `any` escape hatch — and why it defeats the purpose

`any` is not Java's `Object`. `Object` still makes you cast before use. `any` **turns
the type checker off** for that value — every operation on it is allowed, every value
it flows into gets infected, and errors reappear at runtime, which is exactly the
JavaScript you adopted TypeScript to escape.

```ts
const data: any = JSON.parse(response);
data.user.adress.city;        // typo compiles fine; TypeError at runtime — thanks, any

// The right tool: `unknown` — Java-Object-like "could be anything, prove it first"
const parsed: unknown = JSON.parse(response);
// parsed.user;               // COMPILE ERROR: must narrow before use
if (typeof parsed === "object" && parsed !== null && "user" in parsed) { ... }
```

Rules of thumb:
- `unknown` for "I don't know yet" (forces narrowing) — this is your `Object` instinct.
- `any` only at hasty third-party boundaries, quarantined, never in a signature you own.
- Turn on `noImplicitAny` (included in `strict`) so `any` can't sneak in silently.
- `x as SomeType` casts are `(SomeType) x` — but with NO runtime check, ever. A wrong
  `as` doesn't throw ClassCastException; it just lies. Prefer narrowing; validate
  external data (e.g. with `zod`) instead of asserting it.

Grep your own code for `any` and `as` — each one is a hole in the safety net.

---

## Quick-glance rosetta

| Java | TypeScript |
|---|---|
| nominal `implements` | structural — shape is the contract |
| `Optional<T>` | `T \| undefined` + `?.` + `??` |
| sealed classes + switch | discriminated unions + narrowing |
| `final` local | `const` |
| enum | string literal union (usually) |
| `CompletableFuture<T>` | `Promise<T>` + async/await |
| threads + `synchronized` | single-threaded event loop — neither exists |
| Streams | array methods (eager), no bookends |
| DTO / bean class | `interface` + object literal |
| Strategy/Function classes | just pass a function |
| `Object` | `unknown` (never `any`) |
| `(Foo) x` cast | `x as Foo` — compile-time only, no runtime check |
| Maven/Gradle | npm + package.json + tsconfig.json |
| ClassCastException / NPE | prevented by `strict: true` — or reintroduced by `any` |

---

Exercises coming — this track will mirror ramp_up/python/ with starter.ts / solution.ts / vitest.
