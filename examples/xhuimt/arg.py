# pylint: disable=invalid-name,duplicate-code
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

# pylint: disable=import-error,no-name-in-module
from riscv import isa
from riscv.decode import insn_t
from riscv.disasm import xpr_name


__all__ = ["rd", "base_only_address", "rs2"]


@isa.arg
def rd(insn: insn_t) -> str:
    return xpr_name[insn.rd]


@isa.arg
def base_only_address(insn: insn_t) -> str:
    return "(" + xpr_name[insn.rs1] + ")"


@isa.arg
def rs2(insn: insn_t) -> str:
    return xpr_name[insn.rs2]
