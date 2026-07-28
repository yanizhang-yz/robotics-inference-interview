# K Closest Points to Origin

**Difficulty:** Medium  
**Source:** LeetCode 973 "K Closest Points to Origin" — a classic across big tech, commonly reported in AV-company coding screens  
**Tags:** `heap`, `top-k`, `quickselect`, `geometry`

## Problem

Given a list of 2D points and an integer `k`, return the `k` points closest to the
origin `(0, 0)` by Euclidean distance, **in any order**. Also implement the offset
variant `k_closest_to(points, k, target)` — the `k` points closest to an arbitrary
`target` point.

- `k_closest(points, k) -> list of k points`
- `k_closest_to(points, k, target) -> list of k points`

```
k_closest([(1, 3), (-2, 2), (5, 8), (0, 1)], 2)
# -> [(0, 1), (-2, 2)]   (any order)
#    distances: (0,1)=1.0, (-2,2)≈2.83, (1,3)≈3.16, (5,8)≈9.43
```

## Why this appears in robotics inference interviews

This is a per-frame operation in a robot's perception loop. Two everyday examples:

- **Collision checking:** a LiDAR scan hands you thousands of obstacle points; the
  planner only cares about the K nearest ones to the robot.
- **Data association:** a tracker predicts where a pedestrian should be this frame,
  then asks for the K detections nearest that predicted position (`k_closest_to`)
  to decide which detection belongs to which track.

Both run every frame at sensor rate (10–30 Hz), so interviewers want the
O(n log k) or O(n) answer, not a full sort — and they want you to notice the
square-root trick below without prompting.

## Approach

**Compare squared distances, never take the square root.** The squared distance to
`target` is `(x - tx)**2 + (y - ty)**2`. Since `sqrt` is strictly increasing on
non-negative numbers, `sqrt(a) < sqrt(b)` exactly when `a < b` — the *ordering* of
points is identical, so the root buys nothing. Skipping it saves a float op per
point and avoids needless floating-point rounding.

With that key, `heapq.nsmallest(k, points, key=squared_distance)` does the rest: it
internally maintains a bounded heap of the k best candidates while scanning the
list once, which is exactly the "top-k" pattern interviewers are probing for.
`k_closest` is then just `k_closest_to` with `target = (0, 0)`.

Alternatives worth naming out loud: a full sort is O(n log n) — correct but does
more work than needed; **quickselect** partitions around a pivot to find the k-th
distance in O(n) average time (see follow-ups).

**Time:** O(n log k) — one heap operation of cost O(log k) per point.  
**Space:** O(k) for the bounded heap (plus the output).

## Follow-ups to be ready for

1. **Quickselect for O(n) average:** partition the array around a random pivot's
   distance (as in quicksort), then recurse into only the side containing the k-th
   element. Average O(n), worst case O(n²) — randomized pivots make the worst case
   vanishingly unlikely.
2. **Streaming top-k:** if points arrive one at a time (or n doesn't fit in memory),
   keep a max-heap of size k — with `heapq`, store *negated* squared distances —
   and compare each new point against the heap top: O(log k) per point, O(k) memory.
3. **Ties at the k-th distance:** several points can sit at exactly the same
   distance, so the answer set isn't unique. Good tests compare the *multiset of
   distances* rather than exact points; an interviewer may ask you to define a
   deterministic tie-break (e.g., lexicographic on coordinates).
4. **When squared distance is NOT enough:** the trick works because you only ever
   *compare* distances. The moment you need the metric's actual value — a distance
   threshold in meters, averaging distances, mixing with another cost term — you
   must take the square root (or square the threshold instead).
