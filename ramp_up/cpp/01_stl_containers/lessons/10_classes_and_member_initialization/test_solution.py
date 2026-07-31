from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_classes_and_member_initialization_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
