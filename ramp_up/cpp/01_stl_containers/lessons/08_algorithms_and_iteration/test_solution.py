from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_algorithms_and_iteration_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
