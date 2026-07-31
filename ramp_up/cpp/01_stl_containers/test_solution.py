from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_stl_containers_capstone(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE)
