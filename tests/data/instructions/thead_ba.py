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

# pylint: disable=import-error,no-name-in-module
from riscv import insn
from riscv.decode import insn_t
from riscv.disasm import disasm_insn_t
from riscv.processor import insn_desc_t, processor_t

from . import fmt


# pylint: disable=abstract-method
@insn.register("theadba")
class XTheadBa(insn.ISA):
    """
    Functional Mockup of XTheadBa (th.addsl) Instruction

    C.f. https://github.com/XUANTIE-RV/thead-extension-spec/blob/master/xtheadba.adoc
    """

    def get_instructions(self) -> List[insn_desc_t]:
        return [
            insn_desc_t(0x100b, 0xf800707f, * (self.do_addsl, ) * 8)
        ]

    def get_disasms(self) -> List[disasm_insn_t]:
        return [
            disasm_insn_t("th.addsl", 0x100b, 0xf800707f,
                          fmt.rd, fmt.rs1, fmt.rs2, fmt.imm2),
        ]

    def do_addsl(self, p: processor_t, i: insn_t, pc: int) -> int:
        """
        reg[rd] := reg[rs1] + (reg[rs2] << imm2)
        """
        bits = int.from_bytes(i.bits, 'little')
        imm2 = (bits >> 25) & 0b11
        wdata = p.state.XPR[i.rs1] + (p.state.XPR[i.rs2] << imm2)
        p.state.XPR.write(i.rd, wdata)
        return pc + len(i)
