#
# Copyright 2025 WuXi EsionTech Co., Ltd.
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
# pylint: disable=import-error,no-name-in-module
from riscv.csrs import csr_t
from riscv.decode import insn_t
from riscv.disasm import disasm_insn_t
from riscv.extension import extension_t
from riscv.processor import insn_desc_t, processor_t, illegal_instruction

from . import operands as op


class TheadBa(extension_t):
    """
    XTheadBa (th.addsl) Instruction

    th.addsl rd, rs1, rs2, imm2

    C.f. https://github.com/XUANTIE-RV/thead-extension-spec/blob/master/xtheadba.adoc
    """

    # pylint: disable=unused-argument
    def get_instructions(self, proc: processor_t) -> List[insn_desc_t]:
        return [
            insn_desc_t(0x100b, 0xf800707f, *(self._do_th_addsl, ) * 2, *(illegal_instruction, ) * 6),
        ]

    # pylint: disable=unused-argument
    def get_disasms(self, proc: processor_t) -> List[disasm_insn_t]:
        return [
            disasm_insn_t("th.addsl", 0x100b, 0xf800707f, op.rd, op.rs1, op.rs2, op.imm2)
        ]

    # pylint: disable=unused-argument
    def get_csrs(self, proc: processor_t) -> List[csr_t]:
        return []

    # pylint: disable=unused-argument
    def reset(self, proc: processor_t) -> None:
        super().reset(proc)

    def _do_th_addsl(self, p: processor_t, i: insn_t, pc: int) -> int:
        """
        reg[rd] := reg[rs1] + (reg[rs2] << imm2)
        """
        bits = int.from_bytes(i.bits, 'little')
        imm2 = (bits >> 25) & 0b11
        wdata = p.state.XPR[i.rs1] + (p.state.XPR[i.rs2] << imm2)
        p.state.XPR.write(i.rd, wdata)
        p.state.log_reg_write[i.rd << 4] = (wdata, 0)
        return pc + len(i)
