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
from riscv.csrs import csr_t
from riscv.decode import insn_t
from riscv.processor import processor_t


class MyCSR(csr_t):

    def __init__(self, proc: processor_t, addr: int):
        super().__init__(proc, addr)
        self.v = 0x1234

    # pylint: disable=unused-argument
    def verify_permissions(self, insn: insn_t, write: bool) -> None:
        pass

    def read(self) -> int:
        print("read MYCSR", flush=True)
        return self.v

    def unlogged_write(self, val: int) -> None:
        self.v = val
        print("write MYCSR", flush=True)


def test_csr_t(mock_sim):
    p: processor_t = mock_sim.get_core(0)
    csr = MyCSR(p, 0x7c0)
    p.state.add_csr(csr.address, csr)
    assert p.get_csr(csr.address) == 0x1234
    p.put_csr(csr.address, 0x1235)
    assert p.get_csr(csr.address) == 0x1235
