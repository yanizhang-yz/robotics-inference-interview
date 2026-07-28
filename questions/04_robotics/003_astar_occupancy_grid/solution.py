"""A* shortest path on a 2D occupancy grid (0 = free, 1 = obstacle).

4-connectivity, unit step cost, Manhattan-distance heuristic (admissible on
this grid, so the returned path is optimal). Stale heap entries are skipped
on pop instead of doing a decrease-key, which heapq doesn't support.
"""

import heapq
import itertools


def path_length(path: list[tuple[int, int]]) -> int:
    """Number of steps along a path: len(path) - 1."""
    return len(path) - 1


def astar(grid, start: tuple[int, int], goal: tuple[int, int]):
    """Shortest 4-connected path from start to goal inclusive, or None."""
    rows, cols = len(grid), len(grid[0])

    def is_free(cell):
        r, c = cell
        return 0 <= r < rows and 0 <= c < cols and grid[r][c] == 0

    if not is_free(start) or not is_free(goal):
        return None
    if start == goal:
        return [start]

    def h(cell):
        return abs(cell[0] - goal[0]) + abs(cell[1] - goal[1])

    tie = itertools.count()  # breaks f-ties; nodes themselves are never compared
    g_score = {start: 0}
    came_from = {}
    open_heap = [(h(start), next(tie), start)]

    while open_heap:
        f, _, current = heapq.heappop(open_heap)
        if f > g_score[current] + h(current):
            continue  # stale entry: a cheaper route to `current` was pushed later
        if current == goal:
            path = [goal]
            while path[-1] != start:
                path.append(came_from[path[-1]])
            return path[::-1]
        r, c = current
        for neighbor in ((r - 1, c), (r + 1, c), (r, c - 1), (r, c + 1)):
            if not is_free(neighbor):
                continue
            tentative_g = g_score[current] + 1
            if tentative_g < g_score.get(neighbor, float("inf")):
                g_score[neighbor] = tentative_g
                came_from[neighbor] = current
                heapq.heappush(open_heap, (tentative_g + h(neighbor), next(tie), neighbor))

    return None  # open set exhausted: goal unreachable
