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
import types
import importlib
import pytest


def test_metadata():
    riscv = importlib.import_module("riscv")
    assert isinstance(riscv, types.ModuleType)
    assert hasattr(riscv, "__doc__")
    assert hasattr(riscv, "__version__")


@pytest.mark.parametrize("name", [
    "cfg", "csrs", "debug_module", "decode", "devices", "disasm",
    "extension", "isa_parser", "processor", "htif", "simif", "sim",
    "fesvr",
])
def test_pymodule(name):
    mod = importlib.import_module(f"riscv.{name}")
    assert isinstance(mod, types.ModuleType)
