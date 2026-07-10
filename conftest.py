import sys
import importlib.util
from pathlib import Path


def pytest_pycollect_makemodule(module_path, parent):
    """
    Before pytest imports each test_solution.py, load the solution.py
    that lives in the same directory and register it as 'solution' in
    sys.modules. This prevents module-name collisions across questions.
    """
    if Path(module_path).name == "test_solution.py":
        sol_path = Path(module_path).parent / "solution.py"
        if sol_path.exists():
            spec = importlib.util.spec_from_file_location("solution", sol_path)
            mod = importlib.util.module_from_spec(spec)
            sys.modules["solution"] = mod
            spec.loader.exec_module(mod)
    # Returning None lets pytest handle collection normally
