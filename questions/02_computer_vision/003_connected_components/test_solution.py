import numpy as np

from solution import label_components, largest_component_size


def reference_label(grid):
    """Brute-force scan-order flood fill (explicit stack, no scipy).

    Discovers components in the same row-major order as the solution, so
    both the count and the exact labels array must match.
    """
    grid = np.asarray(grid)
    labels = np.zeros(grid.shape, dtype=int)
    if grid.size == 0:
        return labels, 0
    h, w = grid.shape
    count = 0
    for r in range(h):
        for c in range(w):
            if grid[r, c] == 1 and labels[r, c] == 0:
                count += 1
                labels[r, c] = count
                stack = [(r, c)]
                while stack:
                    cr, cc = stack.pop()
                    for nr, nc in ((cr - 1, cc), (cr + 1, cc), (cr, cc - 1), (cr, cc + 1)):
                        if 0 <= nr < h and 0 <= nc < w and grid[nr, nc] == 1 and labels[nr, nc] == 0:
                            labels[nr, nc] = count
                            stack.append((nr, nc))
    return labels, count


class TestLabelComponents:
    def test_empty_grid(self):
        labels, count = label_components([])
        assert count == 0
        assert labels.size == 0

    def test_all_zeros(self):
        labels, count = label_components(np.zeros((3, 4), dtype=int))
        assert count == 0
        np.testing.assert_array_equal(labels, np.zeros((3, 4), dtype=int))

    def test_single_cell(self):
        labels, count = label_components([[1]])
        assert count == 1
        np.testing.assert_array_equal(labels, [[1]])

    def test_diagonal_not_connected(self):
        # The classic trap: diagonal neighbors are SEPARATE components
        # under 4-connectivity.
        labels, count = label_components([[1, 0], [0, 1]])
        assert count == 2
        np.testing.assert_array_equal(labels, [[1, 0], [0, 2]])

    def test_l_shape_single_component(self):
        grid = [[1, 0, 0], [1, 0, 0], [1, 1, 1]]
        labels, count = label_components(grid)
        assert count == 1
        np.testing.assert_array_equal(labels, grid)  # every 1 gets label 1

    def test_donut_single_component(self):
        grid = [[1, 1, 1], [1, 0, 1], [1, 1, 1]]
        labels, count = label_components(grid)
        assert count == 1
        assert labels[1, 1] == 0  # the hole stays background
        np.testing.assert_array_equal(labels, grid)

    def test_scan_order_determinism(self):
        # First component discovered in row-major order gets label 1.
        grid = [[0, 1, 0, 0], [0, 1, 0, 1], [0, 0, 0, 1]]
        labels, count = label_components(grid)
        assert count == 2
        expected = [[0, 1, 0, 0], [0, 1, 0, 2], [0, 0, 0, 2]]
        np.testing.assert_array_equal(labels, expected)

    def test_numpy_and_list_inputs_agree(self):
        grid = [[1, 1, 0], [0, 0, 1], [1, 0, 1]]
        labels_list, count_list = label_components(grid)
        labels_np, count_np = label_components(np.array(grid))
        assert count_list == count_np == 3
        np.testing.assert_array_equal(labels_list, labels_np)

    def test_random_grid_matches_reference(self):
        rng = np.random.default_rng(0)
        grid = (rng.random((50, 50)) < 0.45).astype(int)
        labels, count = label_components(grid)
        ref_labels, ref_count = reference_label(grid)
        assert count == ref_count
        np.testing.assert_array_equal(labels, ref_labels)


class TestLargestComponentSize:
    def test_empty_grid(self):
        assert largest_component_size([]) == 0

    def test_all_zeros(self):
        assert largest_component_size(np.zeros((4, 4), dtype=int)) == 0

    def test_known_mask(self):
        grid = [[1, 1, 0, 1], [1, 1, 0, 1], [0, 0, 0, 1]]
        # Left block has 4 cells, right column has 3.
        assert largest_component_size(grid) == 4

    def test_donut(self):
        assert largest_component_size([[1, 1, 1], [1, 0, 1], [1, 1, 1]]) == 8

    def test_random_grid_matches_reference(self):
        rng = np.random.default_rng(7)
        grid = (rng.random((50, 50)) < 0.5).astype(int)
        ref_labels, ref_count = reference_label(grid)
        expected = int(np.bincount(ref_labels.ravel())[1:].max())
        assert largest_component_size(grid) == expected
