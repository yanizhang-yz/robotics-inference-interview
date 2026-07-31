from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_optional_results_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
