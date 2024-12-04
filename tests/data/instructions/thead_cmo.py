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
from riscv import isa
from riscv.decode import insn_t
from riscv.disasm import disasm_insn_t
from riscv.processor import insn_desc_t, processor_t

from . import operands as op


# pylint: disable=abstract-method
@isa.register("theadcmo")
class XTheadCmo(isa.ISA):
    """
    Functional Mockup of XTheadCmo Instructions

    C.f. https://github.com/XUANTIE-RV/thead-extension-spec/blob/master/xtheadcmo.adoc
    """

    def get_instructions(self) -> List[insn_desc_t]:
        return [
            # th.dcache.call
            insn_desc_t(
                0b0000000_00001_00000_000_00000_0001011,
                0b1111111_11111_11111_111_11111_1111111,
                *(self.do_dcache_call, ) * 8),
            # th.dcache.iall
            insn_desc_t(
                0b0000000_00010_00000_000_00000_0001011,
                0b1111111_11111_11111_111_11111_1111111,
                *(self.do_dcache_iall, ) * 8),
            # th.dcache.ciall
            insn_desc_t(
                0b0000000_00011_00000_000_00000_0001011,
                0b1111111_11111_11111_111_11111_1111111,
                *(self.do_dcache_ciall, ) * 8),
            # th.dcache.cpa
            insn_desc_t(
                0b0000001_01001_00000_000_00000_0001011,
                0b1111111_11111_00000_111_11111_1111111,
                *(self.do_dcache_cpa, ) * 8),
            # th.dcache.ipa
            insn_desc_t(
                0b0000001_01010_00000_000_00000_0001011,
                0b1111111_11111_00000_111_11111_1111111,
                *(self.do_dcache_ipa, ) * 8),
            # th.dcache.cipa
            insn_desc_t(
                0b0000001_01011_00000_000_00000_0001011,
                0b1111111_11111_00000_111_11111_1111111,
                *(self.do_dcache_cipa, ) * 8),
        ]

    def get_disasms(self) -> List[disasm_insn_t]:
        return [
            disasm_insn_t(
                "th.dcache.call",
                0b0000000_00001_00000_000_00000_0001011,
                0b1111111_11111_11111_111_11111_1111111),
            disasm_insn_t(
                "th.dcache.iall",
                0b0000000_00010_00000_000_00000_0001011,
                0b1111111_11111_11111_111_11111_1111111),
            disasm_insn_t(
                "th.dcache.ciall",
                0b0000000_00011_00000_000_00000_0001011,
                0b1111111_11111_11111_111_11111_1111111),
            disasm_insn_t(
                "th.dcache.cpa",
                0b0000001_01001_00000_000_00000_0001011,
                0b1111111_11111_00000_111_11111_1111111,
                op.rs1),
            disasm_insn_t(
                "th.dcache.ipa",
                0b0000001_01010_00000_000_00000_0001011,
                0b1111111_11111_00000_111_11111_1111111,
                op.rs1),
            disasm_insn_t(
                "th.dcache.cipa",
                0b0000001_01011_00000_000_00000_0001011,
                0b1111111_11111_00000_111_11111_1111111,
                op.rs1),
        ]

    # pylint: disable=unused-argument
    def do_dcache_call(self, p: processor_t, i: insn_t, pc: int) -> int:
        """
        TODO: Implement this
        """
        return pc + len(i)

    # pylint: disable=unused-argument
    def do_dcache_iall(self, p: processor_t, i: insn_t, pc: int) -> int:
        """
        TODO: Implement this
        """
        return pc + len(i)

    # pylint: disable=unused-argument
    def do_dcache_ciall(self, p: processor_t, i: insn_t, pc: int) -> int:
        """
        TODO: Implement this
        """
        return pc + len(i)

    # pylint: disable=unused-argument
    def do_dcache_cpa(self, p: processor_t, i: insn_t, pc: int) -> int:
        """
        TODO: Implement this
        """
        return pc + len(i)

    # pylint: disable=unused-argument
    def do_dcache_ipa(self, p: processor_t, i: insn_t, pc: int) -> int:
        """
        TODO: Implement this
        """
        return pc + len(i)

    # pylint: disable=unused-argument
    def do_dcache_cipa(self, p: processor_t, i: insn_t, pc: int) -> int:
        """
        TODO: Implement this
        """
        return pc + len(i)
