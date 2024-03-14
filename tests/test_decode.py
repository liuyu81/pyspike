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
import pytest
# pylint: disable=import-error,no-name-in-module
from riscv.decode import insn_t, insn_length, insn_fetch_all


@pytest.mark.parametrize("raw_insn,exp_len", [
    pytest.param(0x4581, 2, id="c.li/int"),
    pytest.param(b"\x81\x45", 2, id="c.li"),
    pytest.param(0x02828613, 4, id="addi/int"),
    pytest.param(b"\x13\x86\x82\x02", 4, id="addi"),
])
def test_insn_length(raw_insn, exp_len):
    """
    test insn_length() function
    """
    assert insn_length(raw_insn) == exp_len


@pytest.mark.parametrize("raw_insn,exp_seq", [
    pytest.param(b'\x81\x45\x13\x86\x82\x02\x09\xa0', [0x4581, 0x02828613, 0xa009], id="c.li,addi,c.j"),
    pytest.param(b'\x81\x45\x13\x86\x82\x02\x09\xa0\x13', [0x4581, 0x02828613, 0xa009], id="c.li,addi,c.j,<part>"),
])
def test_insn_fetch_all(raw_insn, exp_seq):
    """
    test insn_fetch_all() function
    """
    seq = insn_fetch_all(raw_insn)
    assert seq == exp_seq


@pytest.mark.parametrize("raw_insn,exp_len", [
    pytest.param(b"\x81\x45", 2, id="c.li"),
    pytest.param(b"\x13\x86\x82\x02", 4, id="addi"),
])
def test_insn_t(raw_insn, exp_len):
    """
    test insn_t class construction and comparison
    """
    insn = insn_t(int.from_bytes(raw_insn, "little"))
    assert len(insn) == exp_len
    assert insn == int.from_bytes(raw_insn, "little")
    assert insn == raw_insn
    assert insn == insn_t(raw_insn)
    assert insn.bits == raw_insn
