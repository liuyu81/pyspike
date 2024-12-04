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
# pylint: disable=import-error,no-name-in-module
from riscv.decode import insn_t
from riscv.processor import insn_desc_t, processor_t, illegal_instruction

from instructions import XTheadBa


# pylint: disable=invalid-name
class addi_t:
    """
    addi rd, rs1, imm
    """

    def __call__(self, p: processor_t, i: insn_t, pc: int) -> int:
        """
        reg[rd] := reg[rs1] + i_imm
        """
        p.state.XPR.write(i.rd, p.state.XPR[i.rs1] + i.i_imm)
        return pc + len(i)


def test_insn_desc_t(mock_sim):

    p: processor_t = mock_sim.get_core(0)
    p.state.XPR.reset()

    do_addi = addi_t()
    d = insn_desc_t(0x13, 0x707f, do_addi, *(illegal_instruction, ) * 7)

    assert d.match == 0x13
    assert d.mask == 0x707f

    i = insn_t(b"\x13\x86\x82\x02")
    assert i.rd == 12   # rd <- a2 (x12)
    assert i.rs1 == 5   # rs1 <- t0 (x5)
    assert i.i_imm == 40

    p.state.XPR.write(i.rs1, 20)
    assert p.state.XPR[i.rs1] == 20     # reg[rs1] <- 20
    assert p.state.XPR[i.rd] == 0

    f32i = d.func(32, False, False)
    assert f32i(p, i, 4) == 8
    assert p.state.XPR[i.rd] == 60


def test_register_custom_insn(mock_sim):
    p: processor_t = mock_sim.get_core(0)
    do_addsl = XTheadBa().do_addsl
    i = insn_desc_t(0x100b, 0xf800707f, do_addsl, *(illegal_instruction, ) * 7)
    p.register_custom_insn(i)
