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
from riscv.decode import insn_t
from riscv.isa_parser import isa_parser_t
from riscv.disasm import arg_t, disassembler_t, disasm_insn_t, xpr_name


@pytest.mark.parametrize("raw_insn,exp_name,exp_mnemonic", [
    pytest.param(b"\x81\x45", "c.li", "c.li    a1, 0", id="c.li"),
    pytest.param(b"\x13\x86\x82\x02", "addi", "addi    a2, t0, 40", id="addi"),
])
def test_disassembler_t(raw_insn, exp_name, exp_mnemonic):
    """
    test riscv disassembler
    """
    insn = insn_t(raw_insn)
    isa_parser = isa_parser_t("rv64gcv_zicsr_zifencei_zba_zbb_zbc_zbs", "msu")
    disasm = disassembler_t(isa_parser)
    assert disasm.disassemble(insn) == exp_mnemonic
    assert disasm.lookup(insn) == insn
    assert disasm.lookup(insn).name == exp_name


# pylint: disable=invalid-name
class rvc_imm_t(arg_t):

    def to_string(self, insn: insn_t) -> str:
        return str(insn.rvc_imm)



class rd_t(arg_t):

    def to_string(self, insn: insn_t) -> str:
        return xpr_name[insn.rd]


class rs1_t(arg_t):

    def to_string(self, insn: insn_t) -> str:
        return xpr_name[insn.rs1]


class rs2_t(arg_t):

    def to_string(self, insn: insn_t) -> str:
        return xpr_name[insn.rs2]


class imm2_t(arg_t):

    def to_string(self, insn: insn_t) -> str:
        bits = int.from_bytes(insn.bits, "little")
        return str((bits >> 25) & 0b11)


# pylint: disable=too-many-arguments
@pytest.mark.parametrize("name,match,mask,operands,raw_insn,mnemonic", [
    pytest.param(
        "c.li", 0x4581, 0x0,
        (rd_t(), rvc_imm_t()),
        b"\x81\x45",
        "c.li    a1, 0", id="c.li"),
    pytest.param(
        "th.addsl", 0x100b, 0xf800707f,
        (rd_t(), rs1_t(), rs2_t(), imm2_t()),
        b"\x8b\x12\x73\x04",
        "th.addsl t0, t1, t2, 2", id="th.addsl"),
])
def test_disasm_insn(*, name, match, mask, operands, raw_insn, mnemonic):
    """
    test custom instruction disassembler
    """

    x = disasm_insn_t(name, match, mask, *operands)
    assert x.name == name
    assert x.match == match
    assert x.mask == mask
    assert x.to_string(insn_t(raw_insn)) == mnemonic

    isa_parser = isa_parser_t("rv64gcv_zicsr_zifencei", "msu")
    disasm = disassembler_t(isa_parser)
    disasm.add_insn(x)
    assert disasm.disassemble(insn_t(raw_insn)) == mnemonic
