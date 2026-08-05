from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_rule_of_zero_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
