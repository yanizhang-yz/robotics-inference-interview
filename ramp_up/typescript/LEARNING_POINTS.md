# A Working Programmer's Map to TypeScript

Dense reference card. Every section: a concept you already use, then what TypeScript
does with it. TS looks like a classic curly-brace OO language on the surface — that's
exactly why it bites: the syntax looks familiar while the type system plays by
different rules.

---

## 1. Structural typing vs nominal typing (THE biggest shift)

In a **nominal** type system, a class satisfies an interface only if it explicitly
declares the relationship. TypeScript types are **structural**: **if it has the shape,
it fits.** Nobody has to declare anything.

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
  wrapping, no declaration ceremony.
- Extra properties are fine on variables passed in (`user` above has `age`), but
  object literals passed *directly* get "excess property checks":
  `greet({ name: "x", age: 1 })` errors — a lint-like safety net, not nominal typing.
- Nominal-style branding is possible but rare; embrace shapes.

---

## 2. Type inference — stop writing types the compiler already knows

TypeScript infers aggressively. Writing types the compiler already knows is noise.

```ts
// TypeScript — inferred, still fully type-checked
const scores = new Map<string, number[]>();  // scores: Map<string, number[]>
const name = user.name;                      // name: string
const doubled = [1, 2, 3].map(x => x * 2);   // doubled: number[] — even through lambdas
```

Where you DO write types:
- **Function parameters and public return types** — the API boundary.
- Empty containers where there's nothing to infer: `const xs: string[] = [];`
- That's roughly it. If your locals all have annotations, you're fighting the language.

`as const` bonus: `const dirs = ["N", "E", "S", "W"] as const;` gives the precise type
`readonly ["N", "E", "S", "W"]` instead of `string[]`.

---

## 3. Union types + narrowing

Where class-hierarchy languages reach for a sealed interface hierarchy plus runtime
type checks, TS reaches for a **union of plain shapes** and **narrows** with ordinary
control flow. The compiler tracks `if` branches and refines the type — no casting.

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

Exhaustiveness: add a `default: const _: never = s;` arm — if a new
variant appears, that line stops compiling. Unions also do what class hierarchies
can't: `type Id = string | number;` or `type Direction = "N" | "E" | "S" | "W";`
(string literal unions replace most enums).

---

## 4. Null handling: `strictNullChecks`, `?.`, `??`

With `strictNullChecks: true` (non-negotiable — see tsconfig section), `null` and
`undefined` are **not** members of other types. `string` cannot hold null. Absence must
be declared, and the compiler forces a check — an optional type built into the type
system instead of a wrapper class.

```ts
// TypeScript — `| undefined` in the type IS the optional
function findUser(id: string): User | undefined { ... }

const u = findUser(id);
// u.name;                          // COMPILE ERROR: u is possibly undefined
const city = u?.address?.city ?? "unknown";   // one line, no wrapper type
```

- `?.` — optional chaining: each step short-circuits to `undefined` on absence,
  replacing a chain of manual null checks.
- `??` — nullish coalescing: fall back only when the left side is null/undefined.
  Use `??`, not `||`: `0 || "x"` gives `"x"` (0 is falsy!), `0 ?? "x"` gives `0`.
  `||` on numbers/strings is a classic bug.
- `x!` — non-null assertion: "trust me." The same smell as any unchecked extract —
  the crash comes back at runtime.
- Optional properties: `interface Config { retries?: number }` — the `?` means
  `number | undefined`.
- Two nulls exist: `undefined` (absent — the default, use this) and `null`
  (explicit empty — mostly from JSON/APIs). `??` and `?.` treat both the same;
  `x == null` (double equals, deliberately) catches both.

Nobody uses an optional wrapper class in TS. The union type + `?.`/`??` is the idiom.

---

## 5. `interface` vs `type` alias vs `class` — and why classes are rare

Three keywords where most OO languages have one. The surprise: **`class` is the one
you'll use least.**

| | Use for | Mental model |
|---|---|---|
| `interface` | Object shapes, especially public APIs; can be extended/merged | the classic interface, but purely structural |
| `type` | Unions, intersections, aliases of anything: `type Id = string \| number` | a name for any type expression, however complex |
| `class` | When you need instances with behavior + `instanceof` + private state | the classic class — but see below |

```ts
interface Pose { x: number; y: number; theta: number; }     // shape
type Meters = number;                                        // alias
type Command = MoveCmd | StopCmd | HomeCmd;                  // union — needs `type`

// Where an OO codebase would write a class, TS usually writes a plain object + functions:
const start: Pose = { x: 0, y: 0, theta: 0 };
function advance(p: Pose, d: Meters): Pose {
  return { ...p, x: p.x + d * Math.cos(p.theta), y: p.y + d * Math.sin(p.theta) };
}
```

Why classes are rare: structural typing means you don't need a class to satisfy an
interface; object literals are cheap and typed; functions are first-class so you don't
need strategy classes (just pass a function); and `{ ...p, x: 1 }` spread gives
you immutable-update "withers" for free. Reach for `class` when you genuinely want
encapsulated mutable state + methods (a connection pool, a simulator) — not for DTOs,
not for services that are really just namespaced functions.

`interface` vs `type` in practice: for plain object shapes they're interchangeable;
default to `interface` for public object shapes, `type` for unions and everything else.

---

## 6. `async`/`await` + Promises

A `Promise<T>` is a handle to a value that will arrive later — a future — and
`async/await` is the syntax that makes chained futures read like straight-line code.
The deeper difference from thread-based platforms: **JavaScript is single-threaded.**
One event loop. No threads, no locks, no volatile-style memory rules — and no data
races, because two pieces of your code never run simultaneously. Concurrency =
interleaving awaits, not parallelism.

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

| Concept | TypeScript |
|---|---|
| Future / promise of a value | `Promise<T>` |
| Chain a dependent step | `await` (or `.then(f)` — prefer await) |
| Recover from failure | `try { await ... } catch` |
| Wait for all of several | `await Promise.all([a, b])` — typed tuple result! |
| First to settle / first success | `Promise.race` / `Promise.any` |
| Thread pools | none — the event loop schedules everything |
| Locks / synchronized regions | not needed; no shared-memory parallelism |
| Block until done | doesn't exist — you can't block the loop, only await |

Gotchas: an `async` function always returns a `Promise` (even `async fn(): Promise<void>`);
forgetting `await` gives you a `Promise<T>` where you wanted `T` — the type checker
catches this, read the error; a forgotten await that's *discarded* is a floating promise
(lint rule: `no-floating-promises`); `await` in a plain `for` loop is sequential — use
`Promise.all(items.map(...))` for "parallel" fan-out. CPU-bound work still blocks
everything — there's no thread to hide it on (that's what worker threads are for, rarely).

---

## 7. Generics: familiar syntax, a more capable compile-time system

The `<T>` / `<T extends Base>` syntax is what you expect, and types are erased at
runtime — but TS generics are structural and inferred, and the *compile-time* system
is far more capable.

```ts
// TypeScript
function max<T>(xs: T[], cmp: (a: T, b: T) => number): T { ... }
const m = max([3, 1, 2], (a, b) => a - b);   // T inferred as number — no <> at call site
```

Differences that matter:
- **Inference at call sites**: you almost never write `max<number>(...)`.
- **No wrapper types**: `number[]` just works; no boxed-vs-primitive distinction.
- **Erasure is total**: you can't do `x instanceof T`, and there is no runtime
  type-token workaround — narrow with runtime checks on structure instead.
- **`keyof` and friends**: `function get<T, K extends keyof T>(obj: T, key: K): T[K]`
  — a getter that's type-safe per property name. Also `Partial<T>` (all fields
  optional), `Pick<T, K>`, `Record<K, V>` — types computed *from other types*.
- **Variance is inferred** from usage, not declared: no wildcard annotations to
  memorize. (Arrays/functions behave mostly as you'd hope; you'll rarely
  think about it.)
- Default type params exist: `function make<T = string>(): T[] { return []; }`.

---

## 8. `const`/`let` and scoping — no hoisting surprises (if you do it right)

| Keyword | Meaning | Mental model |
|---|---|---|
| `const` | Block-scoped, no reassignment — **your default** | a constant binding (caveat: object contents still mutable) |
| `let` | Block-scoped, reassignable | a normal local variable |
| `var` | Function-scoped, hoisted, legacy — **never use** | no analogy; a footgun |

```ts
const xs = [1, 2, 3];
xs.push(4);            // fine — const locks the binding, not the array
// xs = [];            // error

for (let i = 0; i < 3; i++) { setTimeout(() => console.log(i)); }  // 0 1 2 — let is per-iteration
// with `var i`, this prints 3 3 3 — the classic JS interview gotcha, avoided by never using var
```

Stick to `const` (reach for `let` only when you actually reassign, e.g. accumulators)
and scoping behaves exactly as block structure suggests. Deep immutability when you
want it: `readonly` properties, `ReadonlyArray<T>`, `as const`.

---

## 9. Array methods: eager, chainable, everywhere

If you have used filter/map/collect pipelines elsewhere, the fluency ports directly —
minus the begin/end bookends. Arrays *are* the pipeline. One real difference: these
are **eager** (each step builds a new array immediately, no lazy fusion). For
interview-sized data, irrelevant.

```ts
// TypeScript — no bookends
const names = users
  .filter(u => u.age >= 18)
  .map(u => u.name)
  .toSorted();          // non-mutating sort (ES2023); .sort() sorts IN PLACE — gotcha
```

| Concept | TS arrays |
|---|---|
| Pipeline begin/end bookends | not needed |
| Transform / filter | `.map(f)` / `.filter(p)` |
| Fold to one value | `.reduce(op, init)` — **note: init comes second** |
| Any / all match | `.some(p)` / `.every(p)` |
| First match | `.find(p)` → `T \| undefined` |
| Flatten nested | `.flatMap(f)` |
| Sorted copy with comparator | `.toSorted(cmp)` — comparator returns a number, `(a,b) => a - b` |
| Take / skip a prefix | `.slice(0, n)` / `.slice(n)` |
| Count matches | `.filter(p).length` |
| Side-effect per element | `.forEach` (or `for...of` — use `of`, never `in`) |
| Group by key | `Object.groupBy(xs, f)` (ES2024) or a reduce into a `Map` |
| Numeric range | `Array.from({ length: n }, (_, i) => i)` (or just a for loop) |
| Dedupe | `[...new Set(xs)]` |

Mutation traps: `.sort()`, `.reverse()`, `.splice()` mutate in place
(use `.toSorted()`, `.toReversed()`, `.slice()`); `.map` on a `Map` doesn't exist —
`Map`/`Set` iterate via `for...of` or `[...map.entries()]` first.

---

## 10. npm + tsconfig

| Task | Node/TS |
|---|---|
| Build manifest | `package.json` (deps + scripts) |
| Package registry | npm registry |
| Install dependencies | `npm install` |
| Dependency scopes | `"dependencies"` / `"devDependencies"` (dev = build/test-only) |
| Version locking | `package-lock.json` — commit it |
| Run tests | `npm test` → runs whatever `"scripts": { "test": ... }` says (vitest here) |
| Run tasks | `npm run <script>` — scripts are the build lifecycle, freeform |
| The compiler | `tsc` — but often only type-CHECKS (`tsc --noEmit`); a bundler or `tsx` actually runs it |
| Language/runtime version | `"target"`/`"lib"` in tsconfig + Node version |
| Compiler flags | `tsconfig.json` — the important file |
| Dependency cache | `node_modules/` per project (yes, it's huge; yes, that's normal) |

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

`any` is not a top type. A proper top type ("could be anything") still makes you prove
what a value is before using it. `any` **turns the type checker off** for that value —
every operation on it is allowed, every value it flows into gets infected, and errors
reappear at runtime, which is exactly the JavaScript you adopted TypeScript to escape.

```ts
const data: any = JSON.parse(response);
data.user.adress.city;        // typo compiles fine; TypeError at runtime — thanks, any

// The right tool: `unknown` — the honest top type: "could be anything, prove it first"
const parsed: unknown = JSON.parse(response);
// parsed.user;               // COMPILE ERROR: must narrow before use
if (typeof parsed === "object" && parsed !== null && "user" in parsed) { ... }
```

Rules of thumb:
- `unknown` for "I don't know yet" (forces narrowing) — the honest top type.
- `any` only at hasty third-party boundaries, quarantined, never in a signature you own.
- Turn on `noImplicitAny` (included in `strict`) so `any` can't sneak in silently.
- `x as SomeType` is a cast with NO runtime check, ever. A wrong
  `as` doesn't throw a cast exception; it just lies. Prefer narrowing; validate
  external data (e.g. with `zod`) instead of asserting it.

Grep your own code for `any` and `as` — each one is a hole in the safety net.

---

## Quick-glance rosetta

| Concept | TypeScript |
|---|---|
| Interface conformance | structural — shape is the contract |
| Absence of a value | `T \| undefined` + `?.` + `??` |
| Closed set of typed variants | discriminated unions + narrowing |
| Constant binding | `const` |
| Enumerations | string literal union (usually) |
| Futures / async results | `Promise<T>` + async/await |
| Threads + locks | single-threaded event loop — neither exists |
| Collection pipelines | array methods (eager), no bookends |
| Plain data record | `interface` + object literal |
| Strategy objects | just pass a function |
| Top type | `unknown` (never `any`) |
| Type cast | `x as Foo` — compile-time only, no runtime check |
| Build + dependency tool | npm + package.json + tsconfig.json |
| Cast/null crashes | prevented by `strict: true` — or reintroduced by `any` |

---

Exercises coming — this track will mirror ramp_up/python/ with starter.ts / solution.ts / vitest.
