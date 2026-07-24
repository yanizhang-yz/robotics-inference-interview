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
        pytest.skip("C++ compiler required for Quest 04")
    return compiler


@pytest.fixture()
def test_binary(tmp_path: Path) -> Path:
    impl = os.environ.get("CPP_QUEST_IMPL", "reference")
    header = f"{impl}.hpp"
    binary = tmp_path / "bounded_blocking_queue_tests"
    compiler = find_compiler()

    command = [
        compiler,
        "-std=c++20",
        "-pthread",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-I",
        str(QUEST_DIR),
        f'-DQUEST_HEADER="{header}"',
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


def test_fifo_single_thread(test_binary: Path) -> None:
    run_case(test_binary, "fifo")


def test_close_drains_existing_items(test_binary: Path) -> None:
    run_case(test_binary, "close_drains")


def test_supports_move_only_values(test_binary: Path) -> None:
    run_case(test_binary, "move_only")


def test_push_blocks_until_space_is_available(test_binary: Path) -> None:
    run_case(test_binary, "blocks_when_full")


def test_multiple_producers_and_consumers(test_binary: Path) -> None:
    run_case(test_binary, "multi_producer_consumer")
