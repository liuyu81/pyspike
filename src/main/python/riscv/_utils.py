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
import ctypes
import importlib.util
import os
import pathlib
import shutil
import sys
import sysconfig
import termios


def find_python_library() -> pathlib.Path:
    """
    Find the absolute path of Python runtime library.
    """
    lib_dir = sysconfig.get_config_var('LIBDIR')
    lib_name = sysconfig.get_config_var('LDLIBRARY')
    multiarch = sysconfig.get_config_var('MULTIARCH')
    dll_ext = sysconfig.get_config_var('SHLIB_SUFFIX')
    candidates = [
        pathlib.Path(lib_dir).joinpath(lib_name).with_suffix(dll_ext),
        pathlib.Path(lib_dir).joinpath(multiarch, lib_name).with_suffix(dll_ext),
    ]
    for lib in candidates:
        if lib.exists():
            return lib.absolute()
    raise RuntimeError("Failed to locate python runtime library (libpython3.so).")


def find_bridge_library() -> pathlib.Path:
    """
    Find the absolute path of pyriscv extension module.
    """
    mod_ext = sysconfig.get_config_var('EXT_SUFFIX')
    dll_ext = sysconfig.get_config_var('SHLIB_SUFFIX')
    candidates = []
    spec = importlib.util.find_spec("riscv")
    if spec is not None and spec.origin is not None:
        candidates.extend([
            pathlib.Path(spec.origin).parent.joinpath(f"_riscv{mod_ext}"),
            pathlib.Path(spec.origin).parent.joinpath(f"_riscv{dll_ext}")
        ])
    candidates.extend([
        pathlib.Path.cwd().joinpath("src", "main", "python", "riscv", f"_riscv{mod_ext}"),
        pathlib.Path.cwd().joinpath("src", "main", "python", "riscv", f"_riscv{dll_ext}"),
    ])

    for lib in candidates:
        if lib.exists():
            return lib.absolute()
    raise RuntimeError("Failed to locate python module (_riscv.so).")


def find_spike_executable(path: str = os.defpath) -> pathlib.Path:
    """
    Find the absolute path of spike executable.
    """
    riscv = os.environ.get("RISCV", "/opt/riscv")
    candidates = [
        pathlib.Path(shutil.which("spike", path=path) or "spike"),
        pathlib.Path(__file__).parent / "data" / "bin" / "spike",
        pathlib.Path(riscv) / "bin" / "spike",
        pathlib.Path("/") / "usr" / "bin" / "spike",
        pathlib.Path("/") / "usr" / "local" / "bin" / "spike",
    ]
    for exe in candidates:
        if exe.exists() and exe.is_file():
            return exe.absolute()
    raise RuntimeError("Failed to locate spike executable (spike).")


def find_spike_library(name: str = "riscv") -> pathlib.Path:
    """
    Find the absolute path of spike runtime library.
    """
    dll_ext = sysconfig.get_config_var('SHLIB_SUFFIX')
    riscv = os.environ.get("RISCV", "/opt/riscv")
    candidates = [
        pathlib.Path(__file__).parent / "data" / "lib" / f"lib{name}{dll_ext}",
        pathlib.Path(riscv) / "lib" / f"lib{name}{dll_ext}",
        pathlib.Path("/") / "usr" / "lib" / f"lib{name}{dll_ext}",
        pathlib.Path("/") / "usr" / "local" / "lib" / f"lib{name}{dll_ext}",
    ]
    for lib in candidates:
        if lib.exists():
            return lib.absolute()
    raise RuntimeError("Failed to locate spike runtime library.")


def load_spike_library(name: str) -> bool:
    """
    Load spike runtime library into the current process.
    """

    # lookup a symbol exposed by spike runtime library (softfloat indeed)
    # to see if we still need to load it into the current process.
    if hasattr(ctypes.CDLL(None), "f32_add"):
        return False

    # pylint: disable=invalid-name
    stdin_fd = None
    old_termios = None

    if sys.stdin.isatty():
        # Get the file descriptor for standard input
        stdin_fd = sys.stdin.fileno()
        # Get the current terminal settings
        old_termios = termios.tcgetattr(stdin_fd)

    ctypes.CDLL(find_spike_library(name).as_posix(), mode=ctypes.RTLD_GLOBAL)

    if stdin_fd is not None and old_termios is not None:
        # Restore the old settings after importing _riscv
        termios.tcsetattr(stdin_fd, termios.TCSAFLUSH, old_termios)

    return True
