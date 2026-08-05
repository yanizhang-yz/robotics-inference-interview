from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_moved_from_state_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
