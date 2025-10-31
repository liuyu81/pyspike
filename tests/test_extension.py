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
from typing import List
from ctypes.util import find_library

import pytest

# pylint: disable=import-error,no-name-in-module
from riscv import isa
from riscv.csrs import csr_t
from riscv.disasm import disasm_insn_t
from riscv.extension import extension_t, register_extension, find_extension
from riscv.processor import insn_desc_t, processor_t


# pylint: disable=unused-argument
class MyCFlush(isa.ISA):

    @property
    def name(self) -> str:
        return "my_cflush"

    def get_instructions(self, proc: processor_t) -> List[insn_desc_t]:
        return []

    def get_disasms(self, proc: processor_t) -> List[disasm_insn_t]:
        return []

    def get_csrs(self, proc: processor_t) -> List[csr_t]:
        return []

    # pylint: disable=useless-parent-delegation
    def reset(self, proc: processor_t):
        super().reset(proc)

    # pylint: disable=useless-parent-delegation
    def set_debug(self, value: bool, proc: processor_t):
        super().set_debug(value, proc)


class MyDummyROCC(isa.ROCC):

    @property
    def name(self) -> str:
        return "my_dummy_rocc"


@pytest.mark.parametrize("name,cls,n_insn,n_disasm", [
    pytest.param("cflush", extension_t, 3, 3, id="cflush"),
    pytest.param("dummy_rocc", extension_t, 4, 0, id="dummy_rocc"),
])
def test_find_extension(mock_sim, name, cls, n_insn, n_disasm):
    if not find_library("customext"):
        pytest.skip("libcustomext.so not found in this build")
    p: processor_t = mock_sim.get_core(0)
    p.reset()
    # lookup
    ext_ctor = find_extension(name)
    assert ext_ctor is not None
    # instantiate
    ext = ext_ctor()
    assert isinstance(ext, cls)
    # instructions
    all_insn = ext.get_instructions(p)
    assert len(all_insn) == n_insn
    for this_insn in all_insn:
        assert isinstance(this_insn, insn_desc_t)
    # disasms
    all_disasm = ext.get_disasms(p)
    assert len(all_disasm) == n_disasm
    for disasm in all_disasm:
        assert isinstance(disasm, disasm_insn_t)
    # reset
    ext.reset(p)


@pytest.mark.parametrize("name,cls,n_insn,n_disasm", [
    pytest.param("my_cflush", MyCFlush, 0, 0, id="my_cflush"),
    pytest.param("my_dummy_rocc", MyDummyROCC, 4, 0, id="my_dummy_rocc"),
])
def test_register_extension(mock_sim, name, cls, n_insn, n_disasm):
    p: processor_t = mock_sim.get_core(0)
    p.reset()
    # register
    register_extension(name, cls)
    # lookup
    ext_ctor = find_extension(name)
    assert ext_ctor is not None
    # instantiate
    ext = ext_ctor()
    assert isinstance(ext, cls)
    assert ext.name == name
    # instructions
    all_insn = ext.get_instructions(p)
    assert len(all_insn) == n_insn
    for this_insn in all_insn:
        assert isinstance(this_insn, insn_desc_t)
    # disasms
    all_disasm = ext.get_disasms(p)
    assert len(all_disasm) == n_disasm
    for disasm in all_disasm:
        assert isinstance(disasm, disasm_insn_t)
    # reset
    ext.reset(p)
