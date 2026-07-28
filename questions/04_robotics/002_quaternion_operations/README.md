# Quaternion Operations

**Difficulty:** Medium  
**Source:** Standard robotics screener — this one is so universal that no single-company attribution applies; expect it anywhere orientation math comes up  
**Tags:** `quaternion`, `rotation`, `SO(3)`, `orientation`, `hamilton-product`

## Problem

Implement the four core quaternion operations. Quaternions are stored **scalar-first** as
`(w, x, y, z)`:

- `quat_multiply(q1, q2)` — the Hamilton product `q1 ⊗ q2` (composition: apply `q2`'s
  rotation first, then `q1`'s).
- `quat_conjugate(q)` — `(w, -x, -y, -z)`; for a unit quaternion this is its inverse.
- `quat_to_rotation_matrix(q)` — the 3×3 rotation matrix for `q`. **Normalize `q`
  inside** so non-unit inputs still produce a valid rotation.
- `rotate_vector(q, v)` — rotate 3D vector `v` by quaternion `q`.

```python
import math
half = math.radians(90) / 2
q = (math.cos(half), 0.0, 0.0, math.sin(half))   # 90° about Z
rotate_vector(q, [1.0, 0.0, 0.0])                # → [0.0, 1.0, 0.0]  (approximately)
```

## Why this appears in robotics inference interviews

Quaternions are how every robotics stack actually stores orientation: ROS
`geometry_msgs/Quaternion`, IMU driver outputs, SLAM pose graphs, robot state
publishers. They win over Euler angles (no gimbal lock) and over raw matrices (4 floats
instead of 9, cheap composition, clean interpolation). Interviewers use this question to
check that you can move between representations — quaternion ↔ matrix ↔ rotated
vector — from the formulas, without reaching for `scipy.spatial.transform`.

## Approach

**Hamilton product.** With `q1 = (w1, x1, y1, z1)` and `q2 = (w2, x2, y2, z2)`:

```
w = w1·w2 − x1·x2 − y1·y2 − z1·z2
x = w1·x2 + x1·w2 + y1·z2 − z1·y2
y = w1·y2 − x1·z2 + y1·w2 + z1·x2
z = w1·z2 + x1·y2 − y1·x2 + z1·w2
```

**Rotation matrix.** Normalize `q` first, then apply the standard `(w, x, y, z)` formula:

```
R = [[1−2(y²+z²),   2(xy−wz),   2(xz+wy)],
     [  2(xy+wz), 1−2(x²+z²),   2(yz−wx)],
     [  2(xz−wy),   2(yz+wx), 1−2(x²+y²)]]
```

**Rotate a vector.** Simplest: build `R` and return `R @ v`. The alternative is the pure
quaternion sandwich `q ⊗ (0, v) ⊗ q*` — same result, worth mentioning out loud in an
interview since it avoids materializing the matrix.

**Time:** O(1) — fixed-size arithmetic. **Space:** O(1).

## Follow-ups

- **SLERP**: how do you interpolate smoothly between two orientations, and why is
  linearly interpolating components (then normalizing) only an approximation?
- **Unit norm**: why must rotation quaternions stay unit-length, and why do integrated
  IMU quaternions drift off the unit sphere (fix: renormalize every step)?
- **Double cover**: `q` and `−q` encode the same rotation — why, and where does this
  bite you (interpolation taking the long way around; sign flips in logs)?
- **Trade-offs**: quaternion vs rotation matrix vs Euler angles — storage, composition
  cost, interpolation, singularities. When would you pick each?
