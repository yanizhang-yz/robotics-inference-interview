from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess

import pytest


QUEST_DIR = Path(__file__).parents[1]
TEST_DRIVER = Path(__file__).with_name("test_driver.cpp")


def find_compiler() -> str:
    requested = os.environ.get("CXX")
    if requested:
        return requested
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("C++ compiler required for Quest 08")
    return compiler


@pytest.fixture()
def test_binary(tmp_path: Path) -> Path:
    impl = os.environ.get("CPP_QUEST_IMPL", "reference")
    binary = tmp_path / "sensor_stream_alignment_tests"
    command = [
        find_compiler(),
        "-std=c++20",
        "-pthread",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-I",
        str(QUEST_DIR),
        f'-DQUEST_HEADER="{impl}.hpp"',
        str(TEST_DRIVER),
        "-o",
        str(binary),
    ]
    subprocess.run(command, check=True, capture_output=True, text=True)
    return binary


def run_case(test_binary: Path, name: str) -> None:
    result = subprocess.run(
        [str(test_binary), name],
        capture_output=True,
        text=True,
        timeout=5,
    )
    assert result.returncode == 0, result.stderr


def test_aligns_latest_pair_within_skew(test_binary: Path) -> None:
    run_case(test_binary, "align")


def test_returns_null_when_no_pair_is_close_enough(test_binary: Path) -> None:
    run_case(test_binary, "no_pair")


def test_capacity_evicts_oldest_samples(test_binary: Path) -> None:
    run_case(test_binary, "capacity")


def test_uses_newest_valid_pair(test_binary: Path) -> None:
    run_case(test_binary, "newest")
