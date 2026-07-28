from collections import deque

import numpy as np
import pytest
from solution import astar, path_length


def bfs_shortest_steps(grid, start, goal):
    """Reference optimal step count via BFS, or None if unreachable."""
    rows, cols = len(grid), len(grid[0])

    def is_free(cell):
        r, c = cell
        return 0 <= r < rows and 0 <= c < cols and grid[r][c] == 0

    if not is_free(start) or not is_free(goal):
        return None
    dist = {start: 0}
    queue = deque([start])
    while queue:
        cell = queue.popleft()
        if cell == goal:
            return dist[cell]
        r, c = cell
        for nb in ((r - 1, c), (r + 1, c), (r, c - 1), (r, c + 1)):
            if is_free(nb) and nb not in dist:
                dist[nb] = dist[cell] + 1
                queue.append(nb)
    return None


def assert_valid_path(grid, path, start, goal):
    """Endpoints correct, every cell free, consecutive cells 4-adjacent."""
    assert path[0] == start
    assert path[-1] == goal
    for r, c in path:
        assert grid[r][c] == 0
    for (r1, c1), (r2, c2) in zip(path, path[1:]):
        assert abs(r1 - r2) + abs(c1 - c2) == 1


def random_grid(rng, size, obstacle_prob):
    grid = (rng.random((size, size)) < obstacle_prob).astype(int)
    grid[0][0] = 0
    grid[size - 1][size - 1] = 0
    return grid.tolist()


class TestBasics:
    def test_straight_corridor(self):
        grid = [[0, 0, 0, 0, 0]]
        path = astar(grid, (0, 0), (0, 4))
        assert path == [(0, 0), (0, 1), (0, 2), (0, 3), (0, 4)]
        assert path_length(path) == 4

    def test_obstacle_forces_detour(self):
        grid = [
            [0, 1, 0],
            [0, 1, 0],
            [0, 0, 0],
        ]
        path = astar(grid, (0, 0), (0, 2))
        assert_valid_path(grid, path, (0, 0), (0, 2))
        assert path_length(path) == 6  # around the wall, not through it

    def test_start_equals_goal(self):
        grid = [[0, 0], [0, 0]]
        path = astar(grid, (1, 1), (1, 1))
        assert path == [(1, 1)]
        assert path_length(path) == 0


class TestUnreachable:
    def test_walled_off_goal_returns_none(self):
        grid = [
            [0, 1, 0],
            [0, 1, 0],
            [0, 1, 0],
        ]
        assert astar(grid, (0, 0), (0, 2)) is None

    def test_blocked_start_returns_none(self):
        grid = [[1, 0], [0, 0]]
        assert astar(grid, (0, 0), (1, 1)) is None

    def test_blocked_goal_returns_none(self):
        grid = [[0, 0], [0, 1]]
        assert astar(grid, (0, 0), (1, 1)) is None


class TestRandomGrids:
    def test_paths_valid_on_random_grids(self):
        rng = np.random.default_rng(0)
        for _ in range(20):
            grid = random_grid(rng, 12, obstacle_prob=0.3)
            start, goal = (0, 0), (11, 11)
            path = astar(grid, start, goal)
            reachable = bfs_shortest_steps(grid, start, goal)
            if path is None:
                assert reachable is None  # never miss an existing path
            else:
                assert_valid_path(grid, path, start, goal)

    def test_optimal_length_matches_bfs(self):
        rng = np.random.default_rng(7)
        checked = 0
        for _ in range(15):
            grid = random_grid(rng, 15, obstacle_prob=0.25)
            start, goal = (0, 0), (14, 14)
            optimal = bfs_shortest_steps(grid, start, goal)
            path = astar(grid, start, goal)
            if optimal is None:
                assert path is None
            else:
                assert_valid_path(grid, path, start, goal)
                assert path_length(path) == optimal  # admissible h -> optimal path
                checked += 1
        assert checked >= 5  # make sure the seed actually exercised solvable grids


class TestLargeMaze:
    def test_40x40_serpentine_maze(self):
        # Horizontal walls with alternating gaps force a snake-shaped path.
        size = 40
        grid = [[0] * size for _ in range(size)]
        for r in range(1, size - 1, 2):
            gap = size - 1 if (r // 2) % 2 == 0 else 0
            for c in range(size):
                if c != gap:
                    grid[r][c] = 1
        start, goal = (0, 0), (size - 1, size - 1)
        path = astar(grid, start, goal)
        assert path is not None
        assert_valid_path(grid, path, start, goal)
        assert path_length(path) == bfs_shortest_steps(grid, start, goal)
