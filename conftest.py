import importlib.util
import os
import sys
from pathlib import Path

# Each question folder contains:
#   starter.py       — your attempt (fill in the TODOs)
#   solution.py      — reference solution
#   test_solution.py — tests, written as `from solution import ...`
#
# By default tests run against the reference. Set PRACTICE=1 to run them
# against your starter.py instead:
#   PRACTICE=1 uv run pytest questions/01_algorithms/001_sliding_window_maximum

PRACTICE_MODE = os.environ.get("PRACTICE") == "1"


def pytest_pycollect_makemodule(module_path, parent):
    """
    Before pytest imports each test_solution.py, load the implementation from
    the same directory and register it as 'solution' in sys.modules. This both
    avoids module-name collisions across question folders and lets PRACTICE=1
    swap in starter.py.
    """
    path = Path(module_path)
    if path.name == "test_solution.py":
        impl = path.parent / ("starter.py" if PRACTICE_MODE else "solution.py")
        if not impl.exists():
            impl = path.parent / "solution.py"
        spec = importlib.util.spec_from_file_location("solution", impl)
        mod = importlib.util.module_from_spec(spec)
        sys.modules["solution"] = mod
        spec.loader.exec_module(mod)
    # Returning None lets pytest handle collection normally
