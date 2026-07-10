from collections import deque


def sliding_window_maximum(nums: list[int], k: int) -> list[int]:
    """
    Returns the maximum value in each sliding window of size k.
    Uses a monotonic decreasing deque of indices — O(n) time, O(k) space.
    """
    if not nums or k == 0:
        return []

    dq: deque[int] = deque()  # stores indices, front = index of current max
    result: list[int] = []

    for i, val in enumerate(nums):
        # Drop indices that have fallen outside the window
        if dq and dq[0] < i - k + 1:
            dq.popleft()

        # Maintain decreasing order: pop smaller values from the back
        while dq and nums[dq[-1]] <= val:
            dq.pop()

        dq.append(i)

        # Start recording once the first full window is complete
        if i >= k - 1:
            result.append(nums[dq[0]])

    return result
