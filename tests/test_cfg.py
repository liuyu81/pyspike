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
from riscv.cfg import mem_cfg_t, cfg_t
from riscv.debug_module import debug_module_config_t


def test_mem_cfg_t():
    mc = mem_cfg_t(0x8000_0000, 0x4000)
    assert mc.base == 0x8000_0000
    assert mc.size == 0x4000


def test_cfg_t():
    c = cfg_t()
    c.isa = "RV64IMAFDCV"
    assert c.isa == "RV64IMAFDCV"


def test_debug_module_config_t():
    dmc = debug_module_config_t()
    dmc.progbufsize = 0
    assert dmc.progbufsize == 0
