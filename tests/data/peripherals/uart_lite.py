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
from enum import Enum
from typing import Optional
from riscv import dev


class Reg(int, Enum):
    RX_FIFO             = 0x00
    TX_FIFO             = 0x04
    STAT_REG            = 0x08
    CTRL_REG            = 0x0C


class CTRLBit(int, Enum):
    RST_TX_FIFO         = 0b0000_0001
    RST_RX_FIFO         = 0b0000_0010
    ENABLE_INTR         = 0b0001_0000


class STATBit(int, Enum):
    RX_FIFO_VALID_DATA  = 0b0000_0001
    RX_FIFO_FULL        = 0b0000_0010
    TX_FIFO_EMPTY       = 0b0000_0100
    TX_FIFO_FULL        = 0b0000_1000
    INTR_ENABLED        = 0b0001_0000
    OVERRUN_ERROR       = 0b0010_0000
    FRAME_ERROR         = 0b0100_0000
    PARITY_ERROR        = 0b1000_0000


class UARTLiteMMIO(dev.MMIO):
    """
    Functional Mockup of Xilinx AMBA UART Lite IP Core
    """

    def __init__(self, sim, args: Optional[str] = None) -> None:
        super().__init__(sim, args)
        self.regs = bytearray(16)
        self.tx_fifo = bytearray()
        self.rx_fifo = bytearray()

    def load(self, addr: int, size: int) -> bytes:
        assert 0 <= addr <= len(self.regs) - size
        regs = memoryview(self.regs)
        # read RX_FIFO
        if addr == Reg.RX_FIFO and size == 4:
            if self.rx_fifo:
                regs[addr] = self.rx_fifo.pop(0)
                regs[Reg.STAT_REG] &= ~STATBit.RX_FIFO_VALID_DATA
        # read STAT_REG
        if addr == Reg.STAT_REG and size == 4:
            if self.rx_fifo:
                regs[addr] |= STATBit.RX_FIFO_VALID_DATA
            if not self.tx_fifo:
                regs[addr] |= STATBit.TX_FIFO_EMPTY
        return regs[addr:addr + size].tobytes()

    def store(self, addr: int, data: bytes) -> None:
        assert 0 <= addr <= len(self.regs) - len(data)
        regs = memoryview(self.regs)
        regs[addr:addr + len(data)] = data
        # write TX_FIFO
        if addr == Reg.TX_FIFO and len(data) == 4:
            value = regs[addr:addr + 4].tobytes()
            self.tx_fifo.append(value[0])
