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
EXAM_DIR = pathlib.Path(__file__).parent.parent / "examples"


@pytest.mark.parametrize("argv,req_resp,ret_code", [
    pytest.param([
        "--isa=rv32imc_zicsr_zifencei_zba_zbb_zbs",
        "--priv=m",
        "-m0x90000000:0x40000000",
        "--pc=0x90000000",
        "--extlib=" + EXAM_DIR.joinpath("peripherals").as_posix(),
        "--device=amba_uartlite:plic,0x20000000",
        DATA_DIR.joinpath("plic-uart_echo.elf").as_posix(),
    ], [
        (None, "warning: tohost and fromhost symbols not in ELF; can't communicate with target"),
        ("hello world!\r\n", "1:hello world!\r\n"),
        ("hello again!\r\n", "1:hello again!\r\n"),
    ], 0, id="plic-uart_echo"),
])
def test_pyspike_cli(argv, req_resp, ret_code):
    proc = pexpect.spawnu("pyspike", argv)
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
    proc.close()


@pytest.mark.parametrize("argv,req_resp,ret_code", [
    pytest.param([
        "--isa=rv64gc_zicsr_zifencei_zba_zbb_zbs_xhuimt",
        "--priv=msu",
        "--extlib=" + EXAM_DIR.joinpath("instructions").as_posix(),
        "-d",
        DATA_DIR.joinpath("huimt_msctlr.elf").as_posix()
    ], [
        # bootrom
        ("", "core   0: 0x0000000000001000 (0x00000297) auipc   t0, 0x0\r\n"),
        ("", "core   0: 0x0000000000001004 (0x02028593) addi    a1, t0, 32\r\n"),
        ("", "core   0: 0x0000000000001008 (0xf1402573) csrr    a0, mhartid\r\n"),
        ("", "core   0: 0x000000000000100c (0x0182b283) ld      t0, 24(t0)\r\n"),
        ("", "core   0: 0x0000000000001010 (0x00028067) jr      t0\r\n"),
        # init
        ("", "core   0: 0x0000000080000000 (0x0010029b) addiw   t0, zero, 1\r\n"),
        ("", "core   0: 0x0000000080000004 (0x00001282) c.slli  t0, 32\r\n"),
        ("", "core   0: 0x0000000080000006 (0x000012fd) c.addi  t0, -1\r\n"),
        # csrw
        ("", "core   0: 0x0000000080000008 (0x7c029073) csrw    unknown_7c0, t0\r\n"
             "read MSCTLR\r\n"
             "write MSCTLR\r\n"),
        # csrr
        ("", "core   0: 0x000000008000000c (0x7c002373) csrr    t1, unknown_7c0\r\n"
             "read MSCTLR\r\n"),
    ], 0, id="huimt-msctlr"),
    pytest.param([
        "--isa=rv64gc_zicsr_zifencei_zba_zbb_zbs_xhuimt",
        "--priv=msu",
        "--extlib=" + EXAM_DIR.joinpath("instructions").as_posix(),
        "-d",
        DATA_DIR.joinpath("huimt_lr_sc.elf").as_posix()
    ], [
        # bootrom
        ("", "core   0: 0x0000000000001000 (0x00000297) auipc   t0, 0x0\r\n"),
        ("", "core   0: 0x0000000000001004 (0x02028593) addi    a1, t0, 32\r\n"),
        ("", "core   0: 0x0000000000001008 (0xf1402573) csrr    a0, mhartid\r\n"),
        ("", "core   0: 0x000000000000100c (0x0182b283) ld      t0, 24(t0)\r\n"),
        ("", "core   0: 0x0000000000001010 (0x00028067) jr      t0\r\n"),
        # lr.w / sc.w
        ("", "core   0: 0x0000000080000000 (0x00000297) auipc   t0, 0x0\r\n"),
        ("", "core   0: 0x0000000080000004 (0x06428293) addi    t0, t0, 100\r\n"),
        ("", "core   0: 0x0000000080000008 (0x1002a32f) lr.w    t1, (t0)\r\n"),
        ("", "core   0: 0x000000008000000c (0x1862a3af) sc.w    t2, t1, (t0)\r\n"),
        # # lr.d / sc.d
        ("", "core   0: 0x0000000080000010 (0x00000297) auipc   t0, 0x0\r\n"),
        ("", "core   0: 0x0000000080000014 (0x05828293) addi    t0, t0, 88\r\n"),
        ("", "core   0: 0x0000000080000018 (0x1002b32f) lr.d    t1, (t0)\r\n"),
        ("", "core   0: 0x000000008000001c (0x1862b3af) sc.d    t2, t1, (t0)\r\n"),
    ], 0, id="huimt-lr_sc")
])
def test_pyspike_cli_debug(argv, req_resp, ret_code):
    proc = pexpect.spawnu("pyspike", argv)
    for inp, out in req_resp:
        assert proc.expect_exact("(spike)") == 0
        if inp is not None:
            proc.sendline(inp)
        assert proc.expect_exact(out) == 0
    proc.sendline("q")
    proc.wait()
    assert proc.exitstatus == ret_code
    proc.close()
