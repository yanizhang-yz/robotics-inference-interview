# 3D Rotation Matrix

**Difficulty:** Medium  
**Source:** Nearly universal in robotics interviews  
**Tags:** `kinematics`, `rotation`, `SO(3)`, `coordinate-transform`

## Problem

Implement functions to build **elementary rotation matrices** around each axis, and compose them to rotate a 3D point.

```python
# Rotate point p by angle_deg around the Z axis
p = [1.0, 0.0, 0.0]
Rz(90°) @ p  →  [0.0, 1.0, 0.0]   (approximately)
```

The three elementary matrices:

```
Rx(θ) = [[1,  0,       0    ],
          [0,  cos θ, -sin θ],
          [0,  sin θ,  cos θ]]

Ry(θ) = [[ cos θ, 0, sin θ],
          [  0,    1,  0   ],
          [-sin θ, 0, cos θ]]

Rz(θ) = [[cos θ, -sin θ, 0],
          [sin θ,  cos θ, 0],
          [  0,     0,    1]]
```

## Why this appears in robotics inference interviews

End-to-end perception pipelines deal constantly with coordinate frame transforms: camera → robot base → world. Candidates must be fluent in rotation matrices, understand that R is orthogonal (R⁻¹ = Rᵀ), and explain the difference between intrinsic and extrinsic rotations.

## Follow-up questions

- Convert a rotation matrix to Euler angles or a quaternion.
- Why do Euler angles suffer from gimbal lock?
- How does TF2 (ROS) represent transforms?
- What is the difference between active and passive rotations?
