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
import contextlib
import glob
import importlib
import os
import pathlib
import site
import subprocess
import tempfile
from setuptools import setup
from setuptools.command.build_py import build_py as _build_py
from setuptools_scm import get_version, ScmVersion
# pylint: disable=import-error
from pybind11.setup_helpers import Pybind11Extension, build_ext as _bulid_ext


RISCV: str = os.environ.get("RISCV", "/opt/riscv")

site.addsitedir(pathlib.Path(__file__).parent)

package = importlib.import_module("riscv")


def _config_bridge() -> Pybind11Extension:
    return Pybind11Extension(
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


# pylint: disable=unused-argument
def _local_scheme(version: ScmVersion) -> str:
    try:
        spike_commit = subprocess.run([
            "git", "-C", "vendor/spike", "rev-parse", "--short", "HEAD"
        ], check=True, capture_output=True, text=True).stdout.strip()
        pyspike_commit = subprocess.run([
            "git", "rev-parse", "--short", "HEAD"
        ], check=True, capture_output=True, text=True).stdout.strip()
        return f"+pyspike.g{pyspike_commit}.spike.g{spike_commit}"
    except (FileNotFoundError, subprocess.CalledProcessError):
        pass
    return "+pyspike.unknown.spike.unknown"


@contextlib.contextmanager
def _dynamic_version(source_dir: pathlib.Path):
    with source_dir.joinpath("VERSION").open("r+") as f_ver:
        default_version = f_ver.read()
        try:
            full_version = getattr(package, "__version__", None)
            if full_version is None or "+" not in full_version:
                full_version = get_version(local_scheme=_local_scheme)
            f_ver.seek(0)
            f_ver.write(f"#define SPIKE_VERSION \"{full_version}\"\n")
            f_ver.truncate()
            yield full_version
        finally:
            f_ver.seek(0)
            f_ver.write(default_version)
            f_ver.truncate()


def _build_spike(package_dir: pathlib.Path) -> None:
    source_dir = package_dir.parent.parent.parent.parent.joinpath("vendor", "spike")
    dest_dir = package_dir.joinpath("data")
    if dest_dir.joinpath("include", "riscv", "encoding.h").exists():
        return
    with tempfile.TemporaryDirectory(prefix="build.", dir=source_dir) as build_dir, _dynamic_version(source_dir):
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

    user_options = _bulid_ext.user_options + [
        ('cov', None, "Enable coverage collection"),
    ]

    def initialize_options(self):
        super().initialize_options()
        self.cov = False

    def finalize_options(self):
        super().finalize_options()
        gcov_compile_args = ["-O0", "--coverage"] if self.cov else []
        gcov_link_args = ["--coverage"] if self.cov else []
        for ext in self.extensions:
            ext.extra_compile_args = list(ext.extra_compile_args) + gcov_compile_args
            ext.extra_link_args = list(ext.extra_link_args) + gcov_link_args

    def build_extensions(self):
        _build_spike(pathlib.Path(package.__file__).parent.absolute())
        return super().build_extensions()


if __name__ == '__main__':
    setup(
        ext_modules=[_config_bridge(), ],
        cmdclass={"build_py": build_py, "build_ext": build_ext})
