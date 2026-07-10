import numpy as np
import pytest
from solution import dequantize, quantize_symmetric_int8


class TestQuantizeSymmetricInt8:
    def test_max_maps_to_127(self):
        tensor = np.array([1.0, -1.0, 0.5])
        q, scale = quantize_symmetric_int8(tensor)
        assert q[0] == 127
        assert q[1] == -127

    def test_zero_tensor(self):
        tensor = np.zeros(5)
        q, scale = quantize_symmetric_int8(tensor)
        assert np.all(q == 0)
        assert scale == 1.0

    def test_output_dtype(self):
        tensor = np.array([0.1, -0.2, 0.3])
        q, _ = quantize_symmetric_int8(tensor)
        assert q.dtype == np.int8

    def test_values_within_int8_range(self):
        tensor = np.random.randn(1000).astype(np.float32)
        q, _ = quantize_symmetric_int8(tensor)
        assert np.all(q >= -128) and np.all(q <= 127)

    def test_scale_is_positive(self):
        tensor = np.array([-3.0, 1.5, 2.0])
        _, scale = quantize_symmetric_int8(tensor)
        assert scale > 0


class TestDequantize:
    def test_round_trip_error_small(self):
        tensor = np.array([0.0, 1.0, -1.0, 0.5, -0.25], dtype=np.float32)
        q, scale = quantize_symmetric_int8(tensor)
        reconstructed = dequantize(q, scale)
        # Quantization error should be well below 1% of the range
        assert np.max(np.abs(reconstructed - tensor)) < 0.02

    def test_output_dtype(self):
        q = np.array([0, 64, -64], dtype=np.int8)
        out = dequantize(q, 0.1)
        assert out.dtype == np.float32
