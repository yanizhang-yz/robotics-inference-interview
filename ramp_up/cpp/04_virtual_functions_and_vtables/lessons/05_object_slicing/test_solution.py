from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_object_slicing_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
