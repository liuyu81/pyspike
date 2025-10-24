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
from typing import Optional
from riscv import dev
from riscv.fesvr import term  # pylint: disable=import-error,no-name-in-module

from .uart_lite import UARTLiteMMIO


@dev.register("amba_uartlite")
class UARTLite(UARTLiteMMIO):
    """
    Functional mockup of Xilinx AMBA UART Lite
    """

    def __init__(self, sim, args: Optional[str] = None):
        super().__init__(sim, args)
        self.rx_fifo.extend(args.encode("utf8") if args else b"")

    def tick(self, rtc_ticks: int) -> None:
        super().tick(rtc_ticks)
        # flush TX_FIFO to stdout
        if self.tx_fifo:
            term.write(bytes(self.tx_fifo))
            self.tx_fifo.clear()
        # load RX_FIFO from stdin
        self.rx_fifo.extend(term.read())


@dev.register('amba_uartlite:plic')
class UARTLitePLIC(UARTLite):
    """
    Functional mockup of Xilinx AMBA UART Lite with PLIC
    """

    def __init__(self, sim, args: Optional[str] = None):
        super().__init__(sim, args)
        self.intr_ttl: int = 0

    def tick(self, rtc_ticks: int) -> None:
        old_len = len(self.rx_fifo)
        super().tick(rtc_ticks)
        new_len = len(self.rx_fifo)
        if new_len > old_len:
            self.sim.plic.set_interrupt_level(1, 1)
            self.intr_ttl = 1
        elif self.intr_ttl > 0:
            self.intr_ttl -= 1
        else:
            self.sim.plic.set_interrupt_level(1, 0)
