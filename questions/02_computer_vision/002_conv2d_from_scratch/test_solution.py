import numpy as np
import pytest

from solution import conv2d, output_shape

IMAGE_4X4 = np.array(
    [
        [1, 2, 3, 4],
        [5, 6, 7, 8],
        [9, 10, 11, 12],
        [13, 14, 15, 16],
    ],
    dtype=float,
)


class TestOutputShape:
    def test_no_padding(self):
        assert output_shape(4, 4, 3, 3) == (2, 2)

    def test_same_padding(self):
        # 3x3 kernel with padding=1, stride=1 preserves spatial size
        assert output_shape(6, 7, 3, 3, stride=1, padding=1) == (6, 7)

    def test_stride_two(self):
        assert output_shape(5, 5, 2, 2, stride=2) == (2, 2)

    def test_kernel_covers_image(self):
        assert output_shape(5, 5, 5, 5) == (1, 1)


class TestConv2d:
    def test_identity_kernel(self):
        identity = np.zeros((3, 3))
        identity[1, 1] = 1.0
        out = conv2d(IMAGE_4X4, identity, padding=1)
        np.testing.assert_allclose(out, IMAGE_4X4)

    def test_box_blur_hand_computed(self):
        # 3x3 window sums on IMAGE_4X4 are [[54, 63], [90, 99]]; /9 blurs them.
        kernel = np.ones((3, 3)) / 9.0
        expected = np.array([[6.0, 7.0], [10.0, 11.0]])
        np.testing.assert_allclose(conv2d(IMAGE_4X4, kernel), expected)

    def test_edge_detect_sign_pattern(self):
        # Vertical step: dark left half, bright right half. Sobel-x must
        # respond positive where the window straddles the step and zero
        # in the flat region.
        step = np.zeros((4, 5))
        step[:, 2:] = 1.0
        sobel_x = np.array([[-1, 0, 1], [-2, 0, 2], [-1, 0, 1]], dtype=float)
        out = conv2d(step, sobel_x)
        assert out.shape == (2, 3)
        assert np.all(out[:, :2] > 0)
        np.testing.assert_allclose(out[:, 2], 0.0)

    def test_stride_two_values(self):
        image = np.arange(25, dtype=float).reshape(5, 5)
        kernel = np.ones((2, 2))
        out = conv2d(image, kernel, stride=2)
        # Non-overlapping 2x2 block sums at (0,0), (0,2), (2,0), (2,2).
        expected = np.array([[12.0, 20.0], [52.0, 60.0]])
        assert out.shape == (2, 2)
        np.testing.assert_allclose(out, expected)

    def test_same_padding_shape(self):
        image = np.arange(42, dtype=float).reshape(6, 7)
        out = conv2d(image, np.ones((3, 3)), stride=1, padding=1)
        assert out.shape == (6, 7)

    def test_non_square_kernel(self):
        image = np.arange(20, dtype=float).reshape(4, 5)
        kernel = np.ones((2, 3))
        expected = np.array(
            [
                [21.0, 27.0, 33.0],
                [51.0, 57.0, 63.0],
                [81.0, 87.0, 93.0],
            ]
        )
        np.testing.assert_allclose(conv2d(image, kernel), expected)

    @pytest.mark.parametrize(
        "h, w, kh, kw, stride, padding",
        [
            (4, 4, 3, 3, 1, 0),
            (5, 5, 2, 2, 2, 0),
            (6, 7, 3, 3, 1, 1),
            (4, 5, 2, 3, 1, 0),
            (7, 6, 3, 2, 2, 1),
            (5, 5, 5, 5, 1, 0),
        ],
    )
    def test_output_shape_matches_conv2d(self, h, w, kh, kw, stride, padding):
        rng = np.random.default_rng(0)
        image = rng.standard_normal((h, w))
        kernel = rng.standard_normal((kh, kw))
        out = conv2d(image, kernel, stride=stride, padding=padding)
        assert out.shape == output_shape(h, w, kh, kw, stride, padding)

    def test_kernel_larger_than_image_raises(self):
        with pytest.raises(ValueError):
            conv2d(np.ones((3, 3)), np.ones((5, 5)))
