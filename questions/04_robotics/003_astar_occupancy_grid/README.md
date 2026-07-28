# A* on an Occupancy Grid

**Difficulty:** Medium  
**Source:** THE robotics planning interview staple — reported across AV and robotics companies' onsites; interview guides for Zoox/Waymo-style loops list grid path-planning as a core coding theme  
**Tags:** `astar`, `path-planning`, `occupancy-grid`, `heap`, `graph-search`

## Problem

Find a shortest path on a 2D occupancy grid, where `0` = free and `1` = obstacle.

- `astar(grid, start, goal)` → list of `(row, col)` cells from `start` to `goal`
  **inclusive**, or `None` if the goal is unreachable (including when `start` or `goal`
  is itself an obstacle). Movement is **4-connected** (up/down/left/right), each step
  costs 1, and the heuristic is **Manhattan distance**.
- `path_length(path)` → number of steps along the path (`len(path) - 1`).

```
grid = [[0, 1, 0],          start = (0, 0), goal = (0, 2)
        [0, 1, 0],
        [0, 0, 0]]

astar(grid, start, goal)
# → [(0,0), (1,0), (2,0), (2,1), (2,2), (1,2), (0,2)]     path_length = 6
```

## Why this appears in robotics inference interviews

The planner running on real robots is a direct descendant of exactly this: ROS
costmaps are occupancy grids, and global planners (A*, Dijkstra, hybrid A*) search
them the same way with fancier costs. Interviewers use it to test two things at once:
heap + hashmap fluency (can you write a priority-queue search cleanly under time
pressure?) and whether you know *why* the heuristic must never overestimate the true
remaining cost — that property, **admissibility**, is what guarantees A* returns an
optimal path. Manhattan distance is admissible on a 4-connected unit-cost grid because
you can never reach the goal in fewer steps than the axis-aligned distance.

## Approach

Best-first search where each cell's priority is `f = g + h`: `g` = cost paid so far,
`h` = admissible estimate of cost remaining.

1. **Open set**: a `heapq` min-heap of `(f, tie_counter, node)`. The incrementing tie
   counter breaks f-ties deterministically and avoids ever comparing nodes.
2. **Bookkeeping**: `g_score` dict (best known cost to each cell) and `came_from` dict
   (each cell's predecessor on that best path).
3. Pop the lowest-f entry. If it is **stale** — its f no longer matches the best known
   g for that node — skip it; we pushed a better entry later instead of doing a
   decrease-key. If it's the goal, stop.
4. For each in-bounds, free 4-neighbor: `tentative_g = g[current] + 1`. If that beats
   the neighbor's best known g, record it, set `came_from`, and push
   `(tentative_g + h(neighbor), next(counter), neighbor)`.
5. **Reconstruct** by walking `came_from` back from the goal and reversing.

**Time:** O(N log N) for N grid cells (each cell pushed at most a few times).  
**Space:** O(N) for the heap and dicts.

## Follow-ups

- **8-connectivity**: diagonal steps cost √2 — and Manhattan distance now
  *overestimates* (breaks admissibility), so switch to **octile distance**
  (`max + (√2−1)·min` of the axis deltas) or Euclidean.
- **Weighted cells / costmaps**: cells carry traversal costs (inflation around
  obstacles) — replace the constant step cost of 1; the heuristic must stay a lower
  bound on the *cheapest* possible remaining cost.
- **Dijkstra** is just A* with `h = 0` — when would you prefer it (many goals, no
  usable heuristic)?
- Why does **greedy best-first** (expand by `h` alone, ignoring `g`) fail to guarantee
  optimal paths?
