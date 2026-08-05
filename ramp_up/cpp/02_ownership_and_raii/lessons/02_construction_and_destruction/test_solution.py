from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_construction_and_destruction_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
