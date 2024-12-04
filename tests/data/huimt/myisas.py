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
from ctypes import c_ulong
from typing import List, Optional
# pylint: disable=import-error,no-name-in-module
from riscv.extension import extension_t
from riscv.decode import insn_t
from riscv.disasm import disasm_insn_t
from riscv.processor import insn_desc_t, processor_t

from . import operands as op


class MyLRSC(extension_t):

    def __init__(self):
        super().__init__()
        self._reserved_address_32: Optional[int] = None
        self._reserved_address_64: Optional[int] = None

    def get_instructions(self) -> List[insn_desc_t]:
        return [
            insn_desc_t(0x1000202f, 0xf9f0707f, *(self._do_lr_32, ) * 8),
            insn_desc_t(0x1800202f, 0xf800707f, *(self._do_sc_32, ) * 8),
            insn_desc_t(0x1000302f, 0xf9f0707f, *(self._do_lr_64, ) * 8),
            insn_desc_t(0x1800302f, 0xf800707f, *(self._do_sc_64, ) * 8),
        ]

    def get_disasms(self) -> List[disasm_insn_t]:
        return [
            disasm_insn_t("lr.w", 0x1000202f, 0xf9f0707f, op.rd, op.base_only_address),
            disasm_insn_t("lr.d", 0x1000302f, 0xf9f0707f, op.rd, op.base_only_address),
            disasm_insn_t("sc.w", 0x1800202f, 0xf800707f, op.rd, op.rs2, op.base_only_address),
            disasm_insn_t("sc.d", 0x1800302f, 0xf800707f, op.rd, op.rs2, op.base_only_address),
        ]

    def reset(self) -> None:
        self._reserved_address_32 = None
        self._reserved_address_64 = None

    def _do_lr_32(self, p: processor_t, i: insn_t, pc: int) -> int:
        """
        x[rd] = LoadReserved32(M[x[rs1]])
        """
        addr: int = p.state.XPR[i.rs1]
        self._reserved_address_32 = addr
        self._reserved_address_64 = None
        abs_addr: int = c_ulong(p.mmu.load_reserved_32(addr)).value
        p.state.XPR.write(i.rd, abs_addr)
        p.state.log_reg_write[i.rd << 4] = (abs_addr, 0)
        return pc + len(i)

    def _do_lr_64(self, p: processor_t, i: insn_t, pc: int) -> int:
        """
        x[rd] = LoadReserved64(M[x[rs1]])
        """
        addr: int = p.state.XPR[i.rs1]
        self._reserved_address_32 = None
        self._reserved_address_64 = addr
        abs_addr: int = c_ulong(p.mmu.load_reserved_64(addr)).value
        p.state.XPR.write(i.rd, abs_addr)
        p.state.log_reg_write[i.rd << 4] = (abs_addr, 0)
        return pc + len(i)

    def _do_sc_32(self, p: processor_t, i: insn_t, pc: int) -> int:
        """
        x[rd] = StoreConditional32(M[x[rs1]], x[rs2])
        """
        addr: int = p.state.XPR[i.rs1]
        data: int = p.state.XPR[i.rs2]
        fail: int = 1
        if addr == self._reserved_address_32:
            fail = 0 if p.mmu.store_conditional_32(addr, data & 0xffffffff) else 1
        self._reserved_address_32 = None
        p.state.XPR.write(i.rd, fail)
        p.state.log_reg_write[i.rd << 4] = (fail, 0)
        return pc + len(i)

    def _do_sc_64(self, p: processor_t, i: insn_t, pc: int) -> int:
        """
        x[rd] = StoreConditional64(M[x[rs1]], x[rs2])
        """
        addr: int = p.state.XPR[i.rs1]
        data: int = p.state.XPR[i.rs2]
        fail: int = 1
        if addr == self._reserved_address_64:
            fail = 0 if p.mmu.store_conditional_64(addr, data) else 1
        self._reserved_address_64 = None
        p.state.XPR.write(i.rd, fail)
        p.state.log_reg_write[i.rd << 4] = (fail, 0)
        return pc + len(i)
