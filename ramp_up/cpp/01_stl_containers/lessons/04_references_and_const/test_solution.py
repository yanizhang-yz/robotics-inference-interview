from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_references_and_const_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
