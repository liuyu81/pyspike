# pylint: disable=invalid-name,duplicate-code
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
from riscv.disasm import arg_t, xpr_name


__all__ = ["rd", "rs1", "rs2", ]


class _rd_t(arg_t):

    def to_string(self, insn: insn_t) -> str:
        return xpr_name[insn.rd]


class _rs1_t(arg_t):

    def to_string(self, insn: insn_t) -> str:
        return xpr_name[insn.rs1]


class _rs2_t(arg_t):

    def to_string(self, insn: insn_t) -> str:
        return xpr_name[insn.rs2]


class _imm2_t(arg_t):

    def to_string(self, insn: insn_t) -> str:
        bits = int.from_bytes(insn.bits, 'little')
        return str((bits >> 25) & 0b11)


rd = _rd_t()

rs1 = _rs1_t()

rs2 = _rs2_t()

imm2 = _imm2_t()
