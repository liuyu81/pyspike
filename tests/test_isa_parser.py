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
import pytest
# pylint: disable=import-error,no-name-in-module
from riscv.isa_parser import zba, zbb, zbc, zbs, isa_parser_t


@pytest.mark.parametrize("isa,priv,exp_xlen,exp_flags", [
    pytest.param("rv64imafdcv_zba_zbb_zbc_zbs", "msu", 64, (
        *(ord(bit) for bit in "IMAFDCV"), zba, zbb, zbc, zbs,
    ), id="rv64gcv_zba_zbb_zbc_zbs/msu"),
    pytest.param("rv32imafdc_zba_zbb_zbc_zbs", "msu", 32, (
        *(ord(bit) for bit in "IMAFDC"), zba, zbb, zbc, zbs,
    ), id="rv32gc_zba_zbb_zbc_zbs/msu"),
    pytest.param("rv32imafdc_xmyisa", "msu", 32, (
        *(ord(bit) for bit in "IMAFDC"), "myisa",
    ), id="rv32gc_xmyisa/msu"),
])
def test_isa_parser_t(isa, priv, exp_xlen, exp_flags):
    """
    test isa_parser_t class construction and membership
    """
    isa_parser = isa_parser_t(isa, priv)
    assert isa_parser.max_xlen == exp_xlen
    assert isa_parser.isa == isa
    assert all(x in isa_parser for x in exp_flags)
