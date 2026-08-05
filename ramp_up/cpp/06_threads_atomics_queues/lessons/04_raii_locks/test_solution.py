from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_raii_locks_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE, extra_flags=("-pthread",), timeout=10)
