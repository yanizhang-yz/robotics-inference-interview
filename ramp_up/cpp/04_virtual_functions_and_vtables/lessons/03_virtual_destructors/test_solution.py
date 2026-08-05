from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_virtual_destructors_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
