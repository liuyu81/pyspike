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


# pylint: disable=invalid-name
class addi_t:
    """
    RVI Add Immediate (addi) Instruction

    addi rd, rs1, imm
    """

    def __call__(self, p: processor_t, i: insn_t, pc: int) -> int:
        """
        reg[rd] := reg[rs1] + i_imm
        """
        wdata = p.state.XPR[i.rs1] + i.i_imm
        p.state.XPR.write(i.rd, wdata)
        p.state.log_reg_write[i.rd << 4] = (wdata, 0)
        return pc + len(i)


# pylint: disable=invalid-name
class th_addsl_t:
    """
    XTheadBa (th.addsl) Instruction

    th.addsl rd, rs1, rs2, imm2

    C.f. https://github.com/XUANTIE-RV/thead-extension-spec/blob/master/xtheadba.adoc
    """

    def __call__(self, p: processor_t, i: insn_t, pc: int) -> int:
        """
        reg[rd] := reg[rs1] + (reg[rs2] << imm2)
        """
        bits = int.from_bytes(i.bits, 'little')
        imm2 = (bits >> 25) & 0b11
        wdata = p.state.XPR[i.rs1] + (p.state.XPR[i.rs2] << imm2)
        p.state.XPR.write(i.rd, wdata)
        p.state.log_reg_write[i.rd << 4] = (wdata, 0)
        return pc + len(i)


def test_register_base_insn(mock_sim):
    p: processor_t = mock_sim.get_core(0)
    p.reset()

    do_addi = addi_t()
    d = insn_desc_t(0x13, 0x707f, do_addi, *(illegal_instruction, ) * 7)
    assert d.match == 0x13
    assert d.mask == 0x707f
    p.register_base_insn(d)

    i = insn_t(0x02828613)
    assert i.rd == 12   # rd <- a2 (x12)
    assert i.rs1 == 5   # rs1 <- t0 (x5)
    assert i.i_imm == 40

    p.state.XPR.write(i.rs1, 20)    # x[rs1] <- 20

    f32i = d.func(32, False, False)
    assert f32i(p, i, 4) == 8
    assert p.state.XPR[i.rd] == 60


def test_register_custom_insn(mock_sim):
    p: processor_t = mock_sim.get_core(0)
    p.reset()

    do_th_addsl = th_addsl_t()
    d = insn_desc_t(0x100b, 0xf800707f, do_th_addsl, *(illegal_instruction, ) * 7)
    assert d.match == 0x100b
    assert d.mask == 0xf800707f
    p.register_custom_insn(d)

    i = insn_t(0x0473128b)
    assert i.rd == 5    # rd <- t0 (x5)
    assert i.rs1 == 6   # rs1 <- t1 (x6)
    assert i.rs2 == 7   # rs2 <- tt (x7)
    imm2 = (int.from_bytes(i.bits, 'little') >> 25) & 0b11
    assert imm2 == 2

    p.state.XPR.write(i.rs1, 60)  # x[rs1] <- 60
    p.state.XPR.write(i.rs2, 1)   # x[rs2] <- 1

    f32i = d.func(32, False, False)
    assert f32i(p, i, 4) == 8
    assert p.state.XPR[i.rd] == 64
