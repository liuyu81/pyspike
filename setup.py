#!/usr/bin/env python
# -*- coding: utf-8 -*-
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
import glob
import importlib
import os
import pathlib
import site
import subprocess
import tempfile
from setuptools import setup
from setuptools.command.build_py import build_py as _build_py
# pylint: disable=import-error
from pybind11.setup_helpers import Pybind11Extension, build_ext as _bulid_ext

RISCV = os.environ.get("RISCV", "/opt/riscv")

site.addsitedir(pathlib.Path(__file__).parent)

package = importlib.import_module("riscv")

bridge_module = Pybind11Extension(
    f"{package.__name__}._riscv",
    glob.glob("src/main/cpp/*.cc"),
    extra_compile_args=[
        "-std=c++2a",
    ],
    define_macros=[
        ("ENV_PYSPIKE_LIBS", f"\"{ package.ENV_PYSPIKE_LIBS }\""),
        ("PYBIND11_DETAILED_ERROR_MESSAGES", "1"),
    ],
    extra_link_args=[
        r"-Wl,-rpath,$ORIGIN/data/lib",
        f"-Wl,-rpath,{RISCV}/lib",
        f"-Lsrc/main/python/{package.__name__}/data/lib",
        "-lriscv",
    ],
    include_dirs=[
        "src/main/cpp",
        f"src/main/python/{package.__name__}/data/include",
        f"{RISCV}/include",
    ]
)


def _build_spike(package_dir: pathlib.Path) -> None:
    source_dir = package_dir.parent.parent.parent.parent.joinpath("vendor", "spike")
    dest_dir = package_dir.joinpath("data")
    if dest_dir.joinpath("include", "riscv", "encoding.h").exists():
        return
    with tempfile.TemporaryDirectory(prefix="build.", dir=source_dir) as build_dir:
        current_dir = os.getcwd()
        try:
            os.chdir(build_dir)
            subprocess.run([f"{source_dir}/configure", f"--prefix={dest_dir}"], env=os.environ, check=True)
            subprocess.run(["make", f"-j{os.cpu_count()}"], env=os.environ, check=True)
            subprocess.run(["make", "install"], env=os.environ, check=True)
        finally:
            os.chdir(current_dir)


class build_py(_build_py):

    def build_package_data(self):
        _build_spike(pathlib.Path(self.get_package_dir(package.__name__)).absolute())
        return super().build_package_data()


class build_ext(_bulid_ext):

    def build_extensions(self):
        _build_spike(pathlib.Path(package.__file__).parent.absolute())
        return super().build_extensions()


setup(
    ext_modules=[bridge_module],
    cmdclass={"build_py": build_py, "build_ext": build_ext}
)
