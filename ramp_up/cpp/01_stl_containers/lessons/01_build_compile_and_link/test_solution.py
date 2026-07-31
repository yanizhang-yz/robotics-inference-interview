from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_build_compile_and_link_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
