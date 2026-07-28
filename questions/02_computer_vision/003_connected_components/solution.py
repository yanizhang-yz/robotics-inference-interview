"""Connected-component labeling on a binary occupancy/segmentation mask.

Iterative BFS (collections.deque) rather than recursion: a single
snake-shaped component in a 1920x1080 mask is ~2M cells, far past
Python's ~1000-frame recursion limit, so recursive flood fill would
raise RecursionError. The labels array doubles as the visited set
(nonzero = visited). Components are numbered 1..count in row-major
discovery order, which makes the labeling deterministic.
"""

from collections import deque

import numpy as np

NEIGHBORS_4 = ((-1, 0), (1, 0), (0, -1), (0, 1))


def label_components(grid) -> tuple[np.ndarray, int]:
    """Label 4-connected components; return (labels, count)."""
    grid = np.asarray(grid)
    labels = np.zeros(grid.shape, dtype=int)
    if grid.size == 0:
        return labels, 0

    h, w = grid.shape
    count = 0
    for r in range(h):
        for c in range(w):
            if grid[r, c] == 1 and labels[r, c] == 0:
                count += 1
                labels[r, c] = count
                queue = deque([(r, c)])
                while queue:
                    cr, cc = queue.popleft()
                    for dr, dc in NEIGHBORS_4:
                        nr, nc = cr + dr, cc + dc
                        if (
                            0 <= nr < h
                            and 0 <= nc < w
                            and grid[nr, nc] == 1
                            and labels[nr, nc] == 0
                        ):
                            labels[nr, nc] = count
                            queue.append((nr, nc))
    return labels, count


def largest_component_size(grid) -> int:
    """Cell count of the largest 4-connected component; 0 if none."""
    labels, count = label_components(grid)
    if count == 0:
        return 0
    return int(np.bincount(labels.ravel())[1:].max())
