from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_values_and_copies_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
