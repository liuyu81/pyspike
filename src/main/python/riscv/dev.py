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
from typing import Optional, Tuple, Type

from riscv.devices import abstract_device_t, device_factory_t, mmio_device_map
from riscv.sim import sim_t


__all__ = ['MMIO', 'register']


class MMIO(abstract_device_t):
    """
    MMIO Abstract Base
    """

    def __init__(self, sim: sim_t, args: Optional[str] = None):
        super().__init__()
        self.sim: sim_t = sim
        self.args: Optional[str] = args

    def tick(self, rtc_ticks: int) -> None:
        pass


def register(name: str, replace: bool = False):
    """
    Decorator for registering MMIO device
    """

    if name in mmio_device_map and not replace:
        raise KeyError(f"Device factory '{name}' already registered")

    def mmio_decorator(mmio_cls: Type[MMIO]):

        class MMIOFactory(device_factory_t):

            # pylint: disable=unused-argument
            def parse_from_fdt(self, fdt, sim: sim_t, *sargs: str) -> Tuple[Optional[abstract_device_t], int]:
                return mmio_cls(sim, ",".join(sargs[1:])), int(sargs[0], 16)

            # pylint: disable=unused-argument
            def generate_dts(self, sim: sim_t, *sargs: str) -> str:
                return ""

        mmio_device_map[name] = MMIOFactory()
        return mmio_cls

    return mmio_decorator
