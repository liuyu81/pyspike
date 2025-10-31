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
import sys
import pytest

# pylint: disable=import-error,no-name-in-module
from riscv.cfg import cfg_t, mem_cfg_t
from riscv.debug_module import debug_module_config_t
from riscv.sim import sim_t


@pytest.fixture(scope="session")
def import_from_data_dir():
    path = pathlib.Path(__file__).parent / "data"
    sys.path.insert(0, path.as_posix())
    return path


@pytest.fixture(scope="session")
def mock_sim():
    yield sim_t(
        cfg=cfg_t(
            isa="rv32gc",
            priv="m",
            mem_layout=[
                mem_cfg_t(0x9000_0000, 0x4_0000)
            ],
            start_pc=0x9000_0000
        ),
        halted=True,
        plugin_device_factories=[],
        args=["pk"],
        dm_config=debug_module_config_t())
