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

import pytest
# pylint: disable=import-error,no-name-in-module
from riscv import isa
from riscv.disasm import disasm_insn_t
from riscv.extension import extension_t, register_extension, find_extension
from riscv.processor import insn_desc_t


class MyCFlush(isa.ISA):

    @property
    def name(self) -> str:
        return "my_cflush"

    def get_instructions(self) -> List[insn_desc_t]:
        return []

    def get_disasms(self) -> List[disasm_insn_t]:
        return []

    def reset(self):
        pass

    def set_debug(self, value: bool):
        pass


class MyDummyROCC(isa.ROCC):

    @property
    def name(self) -> str:
        return "my_dummy_rocc"


@pytest.mark.parametrize("name,cls,n_insn,n_disasm", [
    pytest.param("cflush", extension_t, 3, 3, id="cflush"),
    pytest.param("dummy_rocc", extension_t, 4, 0, id="dummy_rocc"),
])
def test_find_extension(name, cls, n_insn, n_disasm):
    # lookup
    ext_ctor = find_extension(name)
    assert ext_ctor is not None
    # instantiate
    ext = ext_ctor()
    assert isinstance(ext, cls)
    # instructions
    all_insn = ext.get_instructions()
    assert len(all_insn) == n_insn
    for this_insn in all_insn:
        assert isinstance(this_insn, insn_desc_t)
    # disasms
    all_disasm = ext.get_disasms()
    assert len(all_disasm) == n_disasm
    for disasm in all_disasm:
        assert isinstance(disasm, disasm_insn_t)


@pytest.mark.parametrize("name,cls,n_insn,n_disasm", [
    pytest.param("my_cflush", MyCFlush, 0, 0, id="my_cflush"),
    pytest.param("my_dummy_rocc", MyDummyROCC, 4, 0, id="my_dummy_rocc"),
])
def test_register_extension(name, cls, n_insn, n_disasm):
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
    all_insn = ext.get_instructions()
    assert len(all_insn) == n_insn
    for this_insn in all_insn:
        assert isinstance(this_insn, insn_desc_t)
    # disasms
    all_disasm = ext.get_disasms()
    assert len(all_disasm) == n_disasm
    for disasm in all_disasm:
        assert isinstance(disasm, disasm_insn_t)
