# Median of a Data Stream

**Difficulty:** Hard  
**Source:** LeetCode 295 "Find Median from Data Stream" — a widely-asked hard-classic across big tech and AV companies; Waymo's coding rounds are reported to mix LeetCode-style problems with autonomy-framed originals  
**Tags:** `heap`, `two-heaps`, `streaming`, `order-statistics`

## Problem

Numbers arrive one at a time. Design a class `MedianStream` that supports:

- `add(x)` — insert a number from the stream.
- `median()` — return the median of everything added so far.

The **median** is the middle value when all elements are sorted: for an odd count it
is the middle element itself; for an even count it is the average of the two middle
elements. Calling `median()` on an empty stream raises `ValueError`.

```
ms = MedianStream()
ms.add(6);  ms.median()   # -> 6
ms.add(10); ms.median()   # -> 8.0   (average of 6 and 10)
ms.add(2);  ms.median()   # -> 6
ms.add(4);  ms.median()   # -> 5.0   (average of 4 and 6)
```

Target: `add` in O(log n), `median` in O(1). Re-sorting on every query is O(n log n)
per call — that is the naive answer interviewers want you to beat.

## Why this appears in robotics inference interviews

This is the latency-monitoring problem. You log inference latency for every camera
frame and want the running **p50** (median) to display on a dashboard or gate an
alert — updated per frame, in O(log n), not by re-sorting the history. The median is
the right statistic here precisely because it is robust: one 2-second GC pause or a
single pathological frame drags the *mean* way up, but leaves the *median* — your
typical per-frame latency — untouched. Interviewers use this problem to check heap
fluency, invariant reasoning (the two-heap balance), and whether you know why
order statistics beat averages for skewed distributions like latency.

## Approach

Split the stream into two halves around the median, each stored in a **heap**
(a binary tree kept partially ordered so the extreme element is at the top,
push/pop in O(log n)):

- `lower` — a **max-heap** of the smaller half; its top is the largest small value.
- `upper` — a **min-heap** of the larger half; its top is the smallest large value.

Python's `heapq` only provides a min-heap, so `lower` stores **negated** values:
pushing `-x` and reading `-heap[0]` turns a min-heap into a max-heap.

On every `add(x)`:
1. Push `x` onto `lower` if it is ≤ `lower`'s top (or `lower` is empty), else onto `upper`.
2. Rebalance so the sizes differ by at most 1, with the convention that `lower`
   holds the extra element when the count is odd: if `lower` is 2 bigger, move its
   top to `upper`; if `upper` is bigger at all, move its top to `lower`.

`median()` then reads only heap tops: odd count → top of `lower`; even count →
average of the two tops.

**Time:** O(log n) per `add` (a constant number of heap pushes/pops), O(1) per `median`.  
**Space:** O(n) — every element lives in exactly one heap.

## Follow-ups to be ready for

1. **Sliding-window median** (LeetCode 480): now elements also *leave*. Heaps cannot
   delete arbitrary elements efficiently, so you need lazy deletion (a counter of
   "dead" values popped only when they surface at a top) or an indexed/balanced
   structure like `sortedcontainers.SortedList`.
2. **p95/p99 instead of p50:** two heaps track exactly one split point — you can move
   the split (keep `lower` at ~95% of the elements) for a single fixed quantile, but
   arbitrary or multiple exact percentiles require full order statistics over all n
   values. Production latency monitoring accepts approximation: t-digest sketches or
   fixed-width histogram buckets (HDR-histogram style) give tight percentile
   estimates in bounded memory.
3. **Memory cap for an infinite stream:** both heaps grow forever (O(n)). Real
   monitors either window the data (last N frames — back to follow-up 1) or switch
   to the bounded-memory approximate structures from follow-up 2.
