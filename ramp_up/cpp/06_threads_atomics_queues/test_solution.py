from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_threads_atomics_queues_capstone(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE, extra_flags=("-pthread",))
