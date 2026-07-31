"""
A* on an Occupancy Grid — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/04_robotics/003_astar_occupancy_grid -v

Peek at solution.py only after you've tried.

Grid cells: 0 = free, 1 = obstacle. Positions are (row, col) tuples.
"""

import heapq
import itertools


def path_length(path: list[tuple[int, int]]) -> int:
    """Number of steps along a path: len(path) - 1."""
    # TODO: implement
    raise NotImplementedError


def astar(grid, start: tuple[int, int], goal: tuple[int, int]):
    """
    Shortest 4-connected path from start to goal, inclusive, or None.

    Plan:
    1. If start or goal is out of bounds or an obstacle -> None.
       If start == goal -> [start].
    2. Heuristic h(cell) = Manhattan distance to goal:
       abs(r - goal_r) + abs(c - goal_c). Admissible: never overestimates.
    3. Open set = min-heap of (f, tie, node) with tie = next(itertools.count())
       so nodes are never compared. g_score dict {start: 0}, came_from dict.
    4. Loop: pop lowest f. Skip STALE entries (f != g_score[node] + h(node)).
       If node == goal, reconstruct. Else for each of the 4 neighbors that is
       in bounds and free: tentative_g = g + 1; if it beats the neighbor's
       best known g, record g_score + came_from and push.
    5. Reconstruct: walk came_from back from goal, then reverse the list.
    6. Heap exhausted -> None (goal unreachable).
    """
    # TODO: implement
    raise NotImplementedError
