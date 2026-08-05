from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_traversal_and_contiguous_storage_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
