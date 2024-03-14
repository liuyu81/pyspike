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
import pathlib
import pexpect
import pytest


DATA_DIR = pathlib.Path(__file__).parent / "data"


@pytest.mark.parametrize("argv,req_resp,ret_code", [
    pytest.param([
        "--isa=rv32imc_zicsr_zifencei_xtheadba",
        "--priv=m",
        "-m0x90000000:0x40000000",
        "--pc=0x90000000",
        "--extlib=" + DATA_DIR.joinpath("instructions").as_posix(),
        "--extlib=" + DATA_DIR.joinpath("peripherals").as_posix(),
        "--device=amba_uartlite:plic,0x20000000",
        DATA_DIR.joinpath("plic-uart_echo.elf").as_posix(),
    ], [
        (None, "warning: tohost and fromhost symbols not in ELF; can't communicate with target"),
        ("hello world!\r\n", "1:hello world!\r\n"),
        ("hello again!\r\n", "1:hello again!\r\n"),
    ], 0, id="plic"),
])
def test_pyspike_cli(argv, req_resp, ret_code):
    proc = pexpect.spawnu("scripts/pyspike", argv)
    for inp, out in req_resp:
        if inp is not None:
            proc.sendline(inp)
        assert proc.expect(out) == 0
    # enter the interactive mode then quit
    proc.sendcontrol("c")
    assert proc.expect("(spike)") == 0
    proc.sendline("q")
    proc.wait()
    assert proc.exitstatus == ret_code
