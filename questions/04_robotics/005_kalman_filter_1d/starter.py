"""
1-D Kalman Filter (Constant Velocity) — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/04_robotics/005_kalman_filter_1d -v

Peek at solution.py only after you've tried.
"""

import numpy as np


class KalmanFilter1D:
    """
    Constant-velocity Kalman filter over state x = [position, velocity]
    with 2x2 covariance P.

    Plan for __init__:
        Store process_var and measurement_var (R). Set
        x = [initial_pos, initial_vel] and P = initial_uncertainty * I(2).
    """

    def __init__(
        self,
        process_var: float,
        measurement_var: float,
        initial_pos: float = 0.0,
        initial_vel: float = 0.0,
        initial_uncertainty: float = 1000.0,
    ):
        # TODO: implement
        raise NotImplementedError

    @property
    def position(self) -> float:
        """Current position estimate x[0]."""
        # TODO: implement
        raise NotImplementedError

    @property
    def velocity(self) -> float:
        """Current velocity estimate x[1]."""
        # TODO: implement
        raise NotImplementedError

    @property
    def covariance(self) -> np.ndarray:
        """Current 2x2 covariance P."""
        # TODO: implement
        raise NotImplementedError

    def predict(self, dt: float) -> None:
        """
        Propagate the state dt seconds forward through the motion model.

        Plan:
            F = [[1, dt], [0, 1]]
            Q = process_var * [[dt**4 / 4, dt**3 / 2],
                               [dt**3 / 2, dt**2    ]]
            x <- F @ x            (coast at current velocity)
            P <- F @ P @ F.T + Q  (uncertainty GROWS)
        """
        # TODO: implement
        raise NotImplementedError

    def update(self, measurement: float) -> None:
        """
        Fuse in one noisy position measurement z.

        Plan (H = [1, 0], so H P H^T + R is the scalar P[0,0] + R):
            y = z - x[0]                    innovation
            S = P[0, 0] + measurement_var   innovation variance
            K = P @ [1, 0]^T / S            Kalman gain (2x1) — the trust ratio
            x <- x + K * y                  move toward the measurement
            P <- (I - K H) @ P              uncertainty SHRINKS
        Optionally re-symmetrize: P <- (P + P.T) / 2 to kill float drift.
        """
        # TODO: implement
        raise NotImplementedError
