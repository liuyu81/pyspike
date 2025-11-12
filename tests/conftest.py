#
# Copyright 2024 WuXi EsionTech Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
import os
import pathlib
import shutil
import subprocess
import sys
import pytest

# pylint: disable=import-error,no-name-in-module
from riscv.cfg import cfg_t, mem_cfg_t
from riscv.debug_module import debug_module_config_t
from riscv.sim import sim_t


GCOV: bool = bool(os.environ.get("GCOV", None))


@pytest.fixture(scope="session")
def import_from_data_dir():
    path = pathlib.Path(__file__).parent / "data"
    sys.path.insert(0, path.as_posix())
    return path


@pytest.fixture(scope="session")
def mock_sim():
    yield sim_t(
        cfg=cfg_t(
            isa="rv32gc",
            priv="m",
            mem_layout=[
                mem_cfg_t(0x9000_0000, 0x4_0000)
            ],
            start_pc=0x9000_0000
        ),
        halted=True,
        plugin_device_factories=[],
        args=["pk"],
        dm_config=debug_module_config_t())


# pylint: disable=unused-argument
@pytest.hookimpl(tryfirst=True)
def pytest_report_header(config: pytest.Config):
    return f"pyspike: gcov={'on' if GCOV else 'off'}"


@pytest.hookimpl(trylast=True)
def pytest_terminal_summary(terminalreporter, exitstatus: int, config: pytest.Config):
    verbosity = config.getoption("verbose")
    if GCOV and exitstatus == 0:
        _lcov_report(terminalreporter, verbosity)


def _lcov_report(terminalreporter, verbosity: int):
    assert shutil.which("lcov") is not None, "`lcov` not found in $PATH"
    assert shutil.which("lcov-report") is not None, "`lcov-report` not found in $PATH"
    project_dir = pathlib.Path(__file__).parent.parent.absolute()
    lcov_cpp = project_dir.joinpath("_riscv.lcov")
    lcov_py = project_dir.joinpath("riscv.lcov")
    # generate C++ trace file
    lcov_verbosity = ["--quiet"] if verbosity < 2 else []
    subprocess.run([
        "lcov", *lcov_verbosity, "--capture", "--test-name", project_dir.name,
        "--directory", project_dir.joinpath("build").as_posix(), "--base-directory", project_dir.as_posix(),
        "--no-external", "--demangle-cpp", "--ignore-errors", "gcov,mismatch", "-o", lcov_cpp.as_posix()
    ], env=os.environ, cwd=project_dir, check=True)
    subprocess.run([
        "lcov", *lcov_verbosity, "--remove", lcov_cpp.as_posix(), r"**/.venv*/**/*", "-o", lcov_cpp.as_posix()
    ], env=os.environ, cwd=project_dir, check=True)
    subprocess.run([
        "lcov", *lcov_verbosity, "--remove", lcov_cpp.as_posix(), r"**/data/include/**/*", "-o", lcov_cpp.as_posix()
    ], env=os.environ, cwd=project_dir, check=True)
    subprocess.run([
        "lcov", *lcov_verbosity, "--substitute", f"s#{project_dir}/##g",
        "--add-tracefile", lcov_cpp.as_posix(), "-o", lcov_cpp.as_posix()
    ], env=os.environ, cwd=project_dir, check=True)
    # generate report (`lcov --list` on 'ubuntu 24.04' is broken, use custom reporter)
    assert lcov_cpp.exists(), "`_riscv.lcov` not generated"
    assert lcov_py.exists(), "`riscv.lcov` not generated"
    result = subprocess.run([
        "lcov-report", lcov_cpp.as_posix(), lcov_py.as_posix()
    ], env=os.environ, cwd=project_dir, check=True, capture_output=True, text=True)
    terminalreporter.write(result.stdout)
