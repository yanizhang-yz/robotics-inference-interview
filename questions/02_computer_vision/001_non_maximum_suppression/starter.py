"""
Non-Maximum Suppression — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/02_computer_vision/001_non_maximum_suppression -v

Peek at solution.py only after you've tried.
"""


def compute_iou(box_a: list[float], box_b: list[float]) -> float:
    """
    Intersection over Union for two boxes [x1, y1, x2, y2].

    Hint: intersection = clamp(min(x2) - max(x1)) * clamp(min(y2) - max(y1));
    union = area_a + area_b - intersection. Watch out for zero union.
    """
    # TODO: implement
    raise NotImplementedError


def nms(boxes: list[list[float]], scores: list[float], iou_threshold: float = 0.5) -> list[int]:
    """
    Non-Maximum Suppression. Return indices (into the original list) of kept boxes.

    Hint: sort by score descending; greedily keep the best box and drop
    everything whose IoU with it is >= iou_threshold; repeat.
    """
    # TODO: implement
    raise NotImplementedError
