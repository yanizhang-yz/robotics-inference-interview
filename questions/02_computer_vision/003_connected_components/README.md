# Connected Components in a Binary Grid

**Difficulty:** Medium  
**Source:** LeetCode 200 "Number of Islands" family — commonly reported across AV and robotics companies  
**Tags:** `bfs`, `flood-fill`, `grid`, `occupancy-grid`, `segmentation`

## Problem

Label the connected components of a binary occupancy/segmentation mask.

```python
label_components(grid) -> (labels, count)
largest_component_size(grid) -> int
```

`grid` is a list-of-lists or `np.ndarray` of 0/1. `labels` is an int array of the same shape: 0 for background, `1..count` for components, using **4-connectivity** (up/down/left/right — diagonals do NOT connect). Components are numbered in row-major discovery order, so the labeling is deterministic.

```
Input:
  grid = [[1, 1, 0, 0],
          [0, 1, 0, 1],
          [0, 0, 0, 1]]

Output:
  labels = [[1, 1, 0, 0],
            [0, 1, 0, 2],
            [0, 0, 0, 2]]
  count = 2
```

## Why this appears in robotics inference interviews

The grid-flood-fill family (LeetCode 200 "Number of Islands" and its variants) is among the most-reported coding questions across AV and robotics companies, and Waymo interviewers are reported to wrap LeetCode-style problems in autonomy framing. The autonomy version is blob labeling: grouping drivable-area pixels in a segmentation mask, clustering occupied cells in a LiDAR occupancy grid into distinct obstacles, or counting how many separate objects a perception stack is looking at.

## Approach

Scan the grid in row-major order. Each time you hit an unlabeled 1, increment the component count and **BFS** from it with `collections.deque`, stamping the new label on every reachable cell.

- **Iterative, not recursive:** a single snake-shaped component in a 1920x1080 mask is ~2M cells — recursive flood fill blows past Python's ~1000-frame recursion limit (`RecursionError`). BFS with a deque (or DFS with an explicit stack) has no such limit.
- **The labels array is the visited set:** a cell with a nonzero label has been visited, so no separate `visited` structure is needed.
- **Determinism:** numbering components by scan order means the same input always produces the same labels — worth stating in an interview, since it makes the function testable.

`largest_component_size` runs the labeling, then takes the max count over labels 1..count (`np.bincount` on the flattened labels); 0 for an empty or all-background grid.

**Time:** O(h·w) — every cell is enqueued at most once.  
**Space:** O(h·w) for the labels array; the BFS frontier is O(h·w) worst case.

## Follow-ups

- **8-connectivity:** count diagonal neighbors as connected — a one-line change (add the 4 diagonal offsets to the neighbor list). Which pixel-art shapes merge?
- **Union-find (two-pass labeling):** the classic connected-components algorithm from the CV literature — first pass records provisional labels and equivalences, second pass resolves them. Compare constants and memory-access patterns with BFS.
- **Streaming/row-by-row labeling:** if the mask is too large for memory, keep only the previous row's labels and a union-find of active equivalences.
- **Largest-K components:** return the K biggest blobs (heap over component sizes) — e.g. keep the K largest obstacles and drop speckle noise.
