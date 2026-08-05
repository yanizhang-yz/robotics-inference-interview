from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_span_views_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
