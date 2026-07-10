# Non-Maximum Suppression (NMS)

**Difficulty:** Medium  
**Source:** Classic CV interview question — YOLO, Faster R-CNN, any detection pipeline  
**Tags:** `bounding-box`, `IoU`, `greedy`, `object-detection`

## Problem

Given a list of bounding boxes `[x1, y1, x2, y2]` and their confidence scores, implement **Non-Maximum Suppression** to filter out overlapping detections.

```
Input:
  boxes  = [[100,100,210,210], [105,105,215,215], [50,50,150,150], [200,200,300,300]]
  scores = [0.9, 0.75, 0.6, 0.8]
  iou_threshold = 0.5

Output: [0, 3]   ← indices of kept boxes
```

## Why this appears in robotics inference interviews

Every real-time perception stack runs NMS. Candidates are expected to implement it from scratch, optimize its runtime (torchvision's NMS is CUDA-accelerated), and reason about the IoU threshold trade-off (high threshold = more boxes kept; low threshold = aggressive suppression).

## Approach

1. Sort boxes by score descending.
2. Greedily pick the highest-score box; suppress all remaining boxes whose IoU with it exceeds the threshold.
3. Repeat until no boxes remain.

**Time:** O(n²) in the worst case.  
**Space:** O(n).

## Follow-up questions interviewers ask

- How would you parallelize this on GPU?
- What is Soft-NMS and when would you use it?
- How does Batched NMS work across multiple classes?
