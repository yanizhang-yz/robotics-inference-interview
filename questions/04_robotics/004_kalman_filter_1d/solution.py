"""1-D constant-velocity Kalman filter in plain 2x2 numpy.

predict() grows the covariance (motion model + process noise); update()
shrinks it (measurement). The Kalman gain is the trust ratio between the
prediction's uncertainty and the sensor's.
"""

import numpy as np


class KalmanFilter1D:
    """Tracks state x = [position, velocity] with 2x2 covariance P."""

    def __init__(
        self,
        process_var: float,
        measurement_var: float,
        initial_pos: float = 0.0,
        initial_vel: float = 0.0,
        initial_uncertainty: float = 1000.0,
    ):
        self.process_var = float(process_var)
        self.measurement_var = float(measurement_var)
        self.x = np.array([initial_pos, initial_vel], dtype=float)
        self.P = float(initial_uncertainty) * np.eye(2)

    @property
    def position(self) -> float:
        return float(self.x[0])

    @property
    def velocity(self) -> float:
        return float(self.x[1])

    @property
    def covariance(self) -> np.ndarray:
        return self.P.copy()

    def predict(self, dt: float) -> None:
        """Coast forward dt seconds; uncertainty grows by the process noise."""
        F = np.array([[1.0, dt], [0.0, 1.0]])
        # White-noise-acceleration process noise, scaled by process_var.
        Q = self.process_var * np.array(
            [[dt**4 / 4.0, dt**3 / 2.0], [dt**3 / 2.0, dt**2]]
        )
        self.x = F @ self.x          # predict state
        self.P = F @ self.P @ F.T + Q  # predict covariance

    def update(self, measurement: float) -> None:
        """Fuse one noisy position measurement; uncertainty shrinks."""
        H = np.array([[1.0, 0.0]])              # we observe position only
        y = float(measurement) - self.x[0]      # innovation
        S = self.P[0, 0] + self.measurement_var  # innovation variance (scalar H P H^T + R)
        K = self.P @ H.T / S                     # Kalman gain: trust ratio, 2x1
        self.x = self.x + K[:, 0] * y            # move toward the measurement
        self.P = (np.eye(2) - K @ H) @ self.P    # shrink covariance
        self.P = (self.P + self.P.T) / 2.0       # re-symmetrize against float drift
