# Sliding Window Maximum

**Difficulty:** Medium  
**Source:** LeetCode 239 — frequently asked in robotics/sensor-data roles  
**Tags:** `deque`, `sliding-window`, `monotonic-queue`

## Problem

You are given an array of integers `nums` and a sliding window of size `k` that moves from the left to the right one step at a time. Return the maximum value in the window at each position.

```
Input:  nums = [1, 3, -1, -3, 5, 3, 6, 7],  k = 3
Output: [3, 3, 5, 5, 6, 7]
```

## Why this appears in robotics inference interviews

Sensor streams (LiDAR intensity, IMU readings, camera frame rates) often need a rolling maximum or minimum computed in real time with O(n) complexity. Interviewers use this to check whether you can reason about data pipelines on constrained hardware.

## Approach

Use a **monotonic decreasing deque** that stores indices. At each step:
1. Remove indices outside the current window from the front.
2. Remove indices from the back whose values are ≤ current value (they can never be the max).
3. The front of the deque is always the index of the current window's maximum.

**Time:** O(n) — each element is pushed and popped at most once.  
**Space:** O(k) for the deque.
