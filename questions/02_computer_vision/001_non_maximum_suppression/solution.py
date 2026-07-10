import numpy as np


def compute_iou(box_a: list[float], box_b: list[float]) -> float:
    """Intersection over Union for two boxes [x1, y1, x2, y2]."""
    x1 = max(box_a[0], box_b[0])
    y1 = max(box_a[1], box_b[1])
    x2 = min(box_a[2], box_b[2])
    y2 = min(box_a[3], box_b[3])

    inter_w = max(0, x2 - x1)
    inter_h = max(0, y2 - y1)
    intersection = inter_w * inter_h

    area_a = (box_a[2] - box_a[0]) * (box_a[3] - box_a[1])
    area_b = (box_b[2] - box_b[0]) * (box_b[3] - box_b[1])
    union = area_a + area_b - intersection

    return intersection / union if union > 0 else 0.0


def nms(boxes: list[list[float]], scores: list[float], iou_threshold: float = 0.5) -> list[int]:
    """
    Non-Maximum Suppression.
    Returns the indices (in the original list) of boxes to keep.
    """
    if not boxes:
        return []

    # Sort by score descending
    order = sorted(range(len(scores)), key=lambda i: scores[i], reverse=True)
    kept: list[int] = []

    while order:
        best = order[0]
        kept.append(best)
        order = order[1:]

        # Keep only boxes that don't heavily overlap with the chosen one
        order = [i for i in order if compute_iou(boxes[best], boxes[i]) < iou_threshold]

    return kept
