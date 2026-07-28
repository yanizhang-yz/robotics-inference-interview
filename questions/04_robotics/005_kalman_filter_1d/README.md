# 1-D Kalman Filter (Constant Velocity)

**Difficulty:** Medium  
**Source:** State estimation and Kalman filtering are reported as core interview material at Zoox and across AV perception teams (per 2026 interview guides)  
**Tags:** `kalman-filter`, `state-estimation`, `sensor-fusion`, `tracking`, `covariance`

## Problem

Implement a 1-D **constant-velocity** Kalman filter in plain numpy: it tracks the state
`[position, velocity]` and its 2×2 covariance from noisy position measurements.

```python
kf = KalmanFilter1D(process_var=0.1, measurement_var=1.0,
                    initial_pos=0.0, initial_vel=0.0, initial_uncertainty=1000.0)
for z in measurements:
    kf.predict(dt=0.1)     # motion model: coast forward, uncertainty GROWS
    kf.update(z)           # measurement: pull toward z, uncertainty SHRINKS
kf.position, kf.velocity, kf.covariance   # floats / 2x2 array
```

- `predict(dt)` propagates the state through the constant-velocity motion model.
- `update(measurement)` fuses in one noisy position measurement.
- Expose `.position`, `.velocity` (floats) and `.covariance` (2×2 array).
- `initial_uncertainty` (default large, e.g. 1000) seeds the covariance diagonal so the
  first measurement dominates a made-up initial state.

## Why this appears in robotics inference interviews

This is THE sensor-fusion primitive. Every multi-object tracker that smooths noisy
per-frame detections into stable tracks (SORT-style trackers, radar/lidar fusion, ball
trackers, cursor smoothing) has a Kalman filter with exactly this motion model at its
core. Interviewers are not checking that you memorized matrix soup — they are checking
that you understand the rhythm: **predict grows uncertainty** (the world moved and you
didn't look), **update shrinks it** (you looked), and the **Kalman gain is a trust
ratio** between your model and your sensor.

## Approach

State `x = [position, velocity]`, covariance `P` (2×2). Two model matrices:

- State transition `F = [[1, dt], [0, 1]]` — position advances by `velocity·dt`.
- Measurement model `H = [1, 0]` — we only observe position.
- Process noise (white-noise-acceleration model), scaled by `process_var`:
  `Q = process_var · [[dt⁴/4, dt³/2], [dt³/2, dt²]]`.

The five equations, in plain English:

1. `x ← F x` — predict the state: coast forward at the current velocity.
2. `P ← F P Fᵀ + Q` — predict the covariance: uncertainty grows by the process noise.
3. `y = z − H x` — innovation: how far off the measurement is from the prediction.
4. `K = P Hᵀ (H P Hᵀ + R)⁻¹` — Kalman gain: the trust ratio. Prediction uncertain and
   sensor good → K near 1 (believe the sensor); the reverse → K near 0 (believe the model).
5. `x ← x + K y` and `P ← (I − K H) P` — update: move toward the measurement by K,
   and shrink the uncertainty accordingly.

With `H = [1, 0]`, `H P Hᵀ + R` is just the scalar `P[0,0] + measurement_var`, so no
matrix inverse is needed — keep it readable 2×2 numpy.

**Time:** O(1) per step. **Space:** O(1).

## Follow-ups

- **2-D constant velocity**: extend to a 4-state `[x, y, vx, vy]` filter for
  image-plane tracking — F and Q become block-diagonal copies of the 1-D versions.
- **Tuning Q vs R**: large `process_var` / small `measurement_var` → jumpy filter that
  trusts the sensor; the reverse → smooth but laggy filter that trusts the model. How
  would you tune them for a jittery detector vs a slow-moving robot?
- **Missed detections**: what do you do when a frame has no measurement? (Predict
  without update — the covariance keeps growing until the track is reacquired or
  dropped.)
- **When a KF is wrong**: the model assumes linear dynamics and Gaussian noise. For
  nonlinear motion or measurement models, you linearize (EKF) or propagate sigma
  points (UKF) — one-liner each.
