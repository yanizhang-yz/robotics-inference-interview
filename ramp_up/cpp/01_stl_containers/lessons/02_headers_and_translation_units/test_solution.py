from pathlib import Path


HERE = Path(__file__).resolve().parent


def test_headers_and_translation_units_lesson(run_cpp_lesson) -> None:
    run_cpp_lesson(HERE, extra_sources=(HERE / "joint_limits.cpp",))
