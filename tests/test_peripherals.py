# pylint: disable=unused-argument,import-error,no-name-in-module,import-outside-toplevel
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
import os
import tempfile
import pytest
from riscv.test import _test_mmio_load, _test_mmio_store, _test_mmio_tick


@pytest.mark.parametrize("message", [
    "hello world!\n"
])
def test_amba_uart_lite(import_from_data_dir, mock_sim, message):
    from peripherals import UARTLitePLIC
    from peripherals.uart_lite import Reg
    # ctor
    uart = UARTLitePLIC(mock_sim, message)
    assert uart is not None
    assert uart.rx_fifo == message.encode("utf8")
    # rx / load RX_FIFO
    for ch in message:
        raw = _test_mmio_load(uart, Reg.RX_FIFO, 4)
        assert int.from_bytes(raw, "little") == ord(ch)
    _test_mmio_tick(uart, 1)
    assert not uart.rx_fifo
    # tx / store TX_FIFO
    for ch in reversed(message):
        raw = ord(ch).to_bytes(4, 'little')
        _test_mmio_store(uart, Reg.TX_FIFO, raw)
    _test_mmio_tick(uart, 1)
    assert not uart.tx_fifo
    # tick (implicit STDIN to RX_FIFO)
    with tempfile.TemporaryFile("w+b") as f:
        f.write(message.encode("utf8"))
        f.seek(0)
        orig_stdin_fd = os.dup(0)
        try:
            os.dup2(f.fileno(), 0)
            _test_mmio_tick(uart, 1)
            _test_mmio_tick(uart, 1)
        finally:
            os.dup2(orig_stdin_fd, 0)
