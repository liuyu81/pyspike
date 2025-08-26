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
from setuptools import setup
# pylint: disable=import-error
from pybind11.setup_helpers import Pybind11Extension, build_ext

RISCV = os.environ.get("RISCV", "/opt/riscv")

site.addsitedir(pathlib.Path(__file__).parent)

package = importlib.import_module("riscv")

module = Pybind11Extension(
    "riscv._riscv",
    glob.glob("src/main/cpp/*.cc"),
    extra_compile_args=[
        "-std=c++2a",
    ],
    define_macros=[
        ("ENV_PYSPIKE_LIBS", f"\"{ package.ENV_PYSPIKE_LIBS }\""),
        ("PYBIND11_DETAILED_ERROR_MESSAGES", "1"),
    ],
    extra_link_args=[
        f"-Wl,-rpath,{RISCV}/lib",
        f"-L{RISCV}/lib",
        "-lriscv",
    ],
    include_dirs=[
        "src/main/cpp",
        f"{RISCV}/include",
    ]
)

setup(
    ext_modules=[module],
    cmdclass={"build_ext": build_ext}
)
