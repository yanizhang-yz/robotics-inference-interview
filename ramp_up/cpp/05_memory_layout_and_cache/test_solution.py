from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_memory_layout_capstone(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
