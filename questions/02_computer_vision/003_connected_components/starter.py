"""
Connected Components in a Binary Grid — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/02_computer_vision/003_connected_components -v

Peek at solution.py only after you've tried.
"""

import numpy as np


def label_components(grid) -> tuple[np.ndarray, int]:
    """
    Label 4-connected components of a binary grid (list-of-lists or ndarray).

    Return (labels, count): labels is a same-shape int array with 0 for
    background and 1..count for components, numbered in row-major
    discovery order.

    Plan:
    1. grid = np.asarray(grid); labels = np.zeros(grid.shape, dtype=int).
       If grid.size == 0, return (labels, 0).
    2. Scan cells in row-major order. On each unlabeled 1: increment
       count, stamp it, and BFS with collections.deque (iterative —
       recursion would hit RecursionError on 1080p masks), labeling
       each 4-neighbor (up/down/left/right, NOT diagonals) that is a 1
       and still unlabeled.
    3. The labels array doubles as the visited set (nonzero = visited).
    """
    # TODO: implement
    raise NotImplementedError


def largest_component_size(grid) -> int:
    """
    Size (cell count) of the largest 4-connected component; 0 if none.

    Plan: label_components, then max of np.bincount over labels 1..count.
    """
    # TODO: implement
    raise NotImplementedError
