import pytest
from solution import compute_iou, nms


class TestComputeIoU:
    def test_identical_boxes(self):
        assert compute_iou([0, 0, 10, 10], [0, 0, 10, 10]) == pytest.approx(1.0)

    def test_no_overlap(self):
        assert compute_iou([0, 0, 10, 10], [20, 20, 30, 30]) == pytest.approx(0.0)

    def test_half_overlap(self):
        # boxes share a 5×10 strip; total union = 150
        iou = compute_iou([0, 0, 10, 10], [5, 0, 15, 10])
        assert iou == pytest.approx(50 / 150)

    def test_one_inside_other(self):
        # small box fully inside big box: IoU = small_area / big_area
        iou = compute_iou([0, 0, 10, 10], [2, 2, 8, 8])
        assert iou == pytest.approx(36 / 100)


class TestNMS:
    def test_basic(self):
        # box 0 (score 0.9) suppresses box 1 (IoU≈0.84 > 0.5)
        # box 2 has IoU≈0.13 with box 0 → kept; box 3 has no overlap → kept
        boxes = [[100, 100, 210, 210], [105, 105, 215, 215], [50, 50, 150, 150], [200, 200, 300, 300]]
        scores = [0.9, 0.75, 0.6, 0.8]
        kept = nms(boxes, scores, iou_threshold=0.5)
        assert set(kept) == {0, 2, 3}

    def test_no_suppression_needed(self):
        # Non-overlapping boxes — all should survive
        boxes = [[0, 0, 10, 10], [20, 20, 30, 30], [40, 40, 50, 50]]
        scores = [0.9, 0.8, 0.7]
        kept = nms(boxes, scores, iou_threshold=0.5)
        assert set(kept) == {0, 1, 2}

    def test_all_suppressed_except_best(self):
        # All boxes nearly identical — only highest score kept
        box = [0, 0, 10, 10]
        boxes = [box, [0, 0, 10, 10], [0, 0, 10, 10]]
        scores = [0.5, 0.9, 0.7]
        kept = nms(boxes, scores, iou_threshold=0.5)
        assert kept == [1]

    def test_empty_input(self):
        assert nms([], [], iou_threshold=0.5) == []

    def test_single_box(self):
        assert nms([[0, 0, 5, 5]], [0.8]) == [0]
