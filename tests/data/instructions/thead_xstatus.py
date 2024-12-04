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
from riscv.csrs import csr_t, proxy_csr_t
from riscv.disasm import disasm_insn_t
from riscv.processor import insn_desc_t, processor_t


class MXStatusCSR(csr_t):
    """
    MXSTATUS
    """

    def __init__(self, proc: processor_t):
        super().__init__(proc, 0x7c0)
        self.v: int = 0x0000_0000_c0c2_8000

    def read(self) -> int:
        print("read MXSTATUS", flush=True)
        return self.v

    # pylint: disable=unused-argument
    def unlogged_write(self, v: int) -> None:
        print("write MXSTATUS", flush=True)


class SXStatusCSR(proxy_csr_t):
    """
    SXSTATUS
    """

    def __init__(self, proc: processor_t, mxstatus: MXStatusCSR):
        super().__init__(proc, 0x5c0, mxstatus)


# pylint: disable=abstract-method
@isa.register("theadxstatus")
class THeadXStatus(isa.ISA):
    """
    Functional Mockup of MXSTATUS/SXSTATUS CSR

    https://github.com/XUANTIE-RV/thead-extension-spec/blob/master/xtheadsxstatus.adoc
    """

    def get_instructions(self) -> List[insn_desc_t]:
        return []

    def get_disasms(self) -> List[disasm_insn_t]:
        return []

    def reset(self) -> None:
        super().reset()
        # add custom CSRs to processor
        mxstatus = MXStatusCSR(self.p)
        sxstatus = SXStatusCSR(self.p, mxstatus)
        self.p.state.add_csr(mxstatus.address, mxstatus)
        self.p.state.add_csr(sxstatus.address, sxstatus)
