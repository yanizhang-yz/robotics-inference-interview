# C++ Drill Runner — Interactive Lesson Loop

**Date:** 2026-08-04

**Repository:** `robotics-inference-interview`

**Status:** Approved design

**Supersedes in part:** the lesson-delivery format of
`2026-07-30-cpp-ramp-up-curriculum-design.md`. The concept sequence and
module organization from that spec are unchanged; this spec replaces how a
learner moves through a lesson and what the module capstone is.

## Purpose

Learning to code should be mostly coding. The current micro-lessons are
technically sound but front-load 300–650 words of prose before a small
fill-in drill, and module 01 alone is ~5,000 words of reading. This spec
turns the lesson sequence into a game loop modeled on chess-puzzle trainers
and Duolingo: a concept card measured in seconds, an active prediction, an
instantly-checked code drill, and a module capstone that plays like a real
game rather than another worksheet.

## The loop

One rep, run by a terminal program invoked as `./drill` from the repo root:

1. **Card** — the runner prints an ~8-line concept card (one code snippet
   allowed). No scrolling walls.
2. **Predict** — one multiple-choice question answered in the terminal
   before any code is written. A wrong pick shows a one-line "why that's
   wrong" for that specific choice and re-prompts; first-try correctness is
   recorded.
3. **Drill** — the runner prints the starter file path and enters watch
   mode: every save of the file triggers compile + run of the real C++
   through the same flags as the pytest harness, repainting a green/red
   verdict in place. The learner edits in whatever editor they like.
4. **Pass** — on green, the runner shows the lesson's 2–3 takeaway bullets
   and advances.

### Escalating hints on failure

- **Fail 1:** the cleaned-up failure only — the first compiler error lines,
  or the failing assertion and surrounding output. Never the raw wall.
- **Fail 2:** the card's deep-dive section unfolds.
- **Fail 3+:** the runner offers the solution diff; the learner chooses.
- The drill always stays retryable until green — like retrying a puzzle.

## Commands

| Command | Behavior |
| --- | --- |
| `./drill` | Resume: serve the next incomplete lesson on the rail; capstone once all 10 pass. |
| `./drill <n>` | Jump to lesson `n` (1–10) — no locks. |
| `./drill capstone` | Jump to the capstone (warns if lessons are incomplete, but does not block). |
| `./drill review` | Re-serve up to 3 previously-passed drills with the starter reset (the passing version is saved to `.drill/solutions/` first). Lightweight spaced repetition. |
| `./drill status` | Progress map, streak, and predict accuracy. |
| `./drill doctor` | Maintainer/setup check: compiler present, every lesson parses, every reference solution passes. |

Non-interactive flags (`--check`, `--answer <A|B|C>`, `--show-solution`)
exist for scripted testing and CI; they are not part of the learner story.

## State

`.drill/` at the repo root, gitignored:

- `progress.json` — per-lesson: passed, attempts, first-pass date,
  first-try predict correctness; plus the set of days played (drives the
  streak display).
- `solutions/` — the learner's passing starter files, saved before a
  `review` reset so nothing is ever lost.

Ordering is a linear rail by default (plain `./drill` = next incomplete),
free jumping by argument, and the capstone is soft-gated: earned, not
locked.

## Lesson file format

One README per lesson stays the single source of truth. The runner parses
these sections; everything else is ignored:

```markdown
# NN — Title

## Card
~8 lines, one small code snippet allowed. The only reading a rep requires.

## Predict
One question ending in "?"

- A) choice
- B) choice
- C) choice

<!-- predict
answer: B
why-A: one line shown when A is picked.
why-B: one line shown on the correct answer.
why-C: one line shown when C is picked.
-->

## Drill
1–3 lines: what to implement in starter.cpp, plus the manual fallback
command for people not using the runner.

## Takeaway
- 2–3 bullets shown after the drill passes.

## Deep dive
The condensed long-form explanation (~150–250 words). Shown on demand
(hint level 2) and readable when browsing GitHub. This is where the old
README prose survives, trimmed.
```

The predict answer key lives in an HTML comment: invisible when rendered,
discoverable in raw view — the same openness as `solution.cpp` sitting in
the same folder. Lesson drills are compiled as `starter.cpp` plus every
other non-`solution.cpp` `.cpp` file in the lesson directory (which is how
lesson 02's `joint_limits.cpp` already works under pytest).

## Capstone: the real game

The module-level fill-in drill (six independent STL contracts) is replaced
by `capstone/` — a small real program written from a near-empty file, in
the module's robotics domain, with **staged tests that unlock
progressively**. The learner runs the whole game: read the stage spec,
build it, watch the stage go green, next stage appears.

Module 01's capstone is a telemetry triage CLI (`capstone/`): stage 1
parses a joint-telemetry stream (strings, vectors, skipping malformed
lines), stage 2 groups and clamps per joint (maps, mutable references),
stage 3 reports uniques and hottest joints (sets, sorting, top-K). The
stages deliberately absorb the string/map/set coverage the old six
contracts provided, so no learning objective is dropped. Stage selection is
`argv[1]`; fixtures are exact stdin→stdout comparisons, verified both by
the runner and by `test_solution.py` under the existing pytest harness.

## Runner implementation constraints

- Single Python file at the repo root (`drill`), executable, stdlib only,
  Python 3.9-compatible (the macOS system interpreter). No pytest, no
  third-party watchers — watch mode is mtime polling.
- Compile flags and compiler discovery are imported from
  `ramp_up/cpp/cpp_test_support.py`; the flags have exactly one home.
- The runner never edits learner files except the explicit `review` reset,
  which archives the current file first.

## Scope

This effort: the runner + all 10 module-01 lessons converted + the staged
capstone. Modules 02–06 convert in follow-up sessions **after** the loop
has been play-tested end to end on module 01. Definition of done for the
pilot: a learner (the repo owner) plays module 01 from lesson 01 through
the capstone without opening a lesson README by hand.

## Explicit non-goals

- No web app, no server, no accounts.
- No Anki-style scheduled repetition — `review` is manual, small, and
  random among passed lessons.
- No hard locks on lesson order.
- No Claude dependency in the core loop (a Claude-driven interview layer
  can be added later without changing this design).
