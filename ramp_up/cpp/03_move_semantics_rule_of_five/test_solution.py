from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_move_semantics_capstone(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
