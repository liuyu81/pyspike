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
from riscv.processor import processor_t
from riscv.fesvr import term


class MSCTLR(csr_t):
    """
    MSCTLR
    """

    def __init__(self, proc: processor_t):
        super().__init__(proc, 0x7c0)
        self.v: int = 0x0000_0000_0000_0000

    def read(self) -> int:
        term.write(b"read MSCTLR\n")
        return self.v

    def write(self, v: int) -> None:
        term.write(b"write MSCTLR\n")
        self.v = v
