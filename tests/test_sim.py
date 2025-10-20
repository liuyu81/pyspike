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
import pathlib
import signal

import pexpect
import pexpect.fdpexpect
import pytest

# pylint: disable=import-error,no-name-in-module
from riscv.cfg import cfg_t, mem_cfg_t
from riscv.debug_module import debug_module_config_t
from riscv.sim import sim_t

# pylint: disable=unused-import
import instructions
import peripherals


DATA_DIR = pathlib.Path(__file__).parent / "data"


@pytest.mark.timeout(3)
@pytest.mark.parametrize("kwargs,req_resp,ret_code", [
    pytest.param({
        "cfg": cfg_t(
            isa="rv32imc_zicsr_zifencei_zba_zbb_zbs",
            priv="m",
            mem_layout=[
                mem_cfg_t(0x9000_0000, 0x4_0000)
            ],
            start_pc=0x9000_0000,
        ),
        "halted": False,
        "plugin_device_factories": [
            ("amba_uartlite:plic", ("0x20000000", )),
        ],
        "args": [
            DATA_DIR.joinpath("plic-uart_echo.elf").as_posix()
        ],
        "dm_config": debug_module_config_t()
    }, [
        (None, "warning: tohost and fromhost symbols not in ELF; can't communicate with target\r\n"),
        ("hello world!\r\n", "1:hello world!\r\n"),
        ("hello again!\r\n", "1:hello again!\r\n"),
    ], 0, id="plic")
])
def test_sim_run(kwargs, req_resp, ret_code):
    s = sim_t(**kwargs)
    pid, fd = os.forkpty()
    if pid == 0:
        s.run()
    # connect to the child process
    proc = pexpect.fdpexpect.fdspawn(fd)
    for inp, out in req_resp:
        if inp is not None:
            proc.sendline(inp)
        assert proc.expect(out) == 0
    # enter the interactive mode then quit
    os.kill(pid, signal.SIGINT)
    assert proc.expect("(spike)") == 0
    proc.sendline("q")
    _, status = os.waitpid(pid, 0)
    assert os.WIFEXITED(status)
    assert os.WEXITSTATUS(status) == ret_code
    proc.close()  # closes fd internally
