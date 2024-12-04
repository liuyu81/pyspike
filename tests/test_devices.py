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
from typing import Optional, Tuple
import pytest

# pylint: disable=import-error,no-name-in-module
from riscv.devices import abstract_device_t, device_factory_t, plic_t, mmio_device_map
from riscv.sim import sim_t
from riscv.test import _test_mmio_load, _test_mmio_store, _test_mmio_parse_from_fdt


class MyDevice(abstract_device_t):

    def __init__(self, sim: sim_t):
        super().__init__()
        self.sim: sim_t = sim

    # pylint: disable=unused-argument
    def load(self, addr: int, size: int) -> bytes:
        return b"hello world!\n"

    # pylint: disable=unused-argument
    def store(self, addr: int, data: bytes) -> None:
        assert data == b"hello again!\n"


class MyFactory(device_factory_t):

    # pylint: disable=unused-argument
    def parse_from_fdt(self, fdt, sim: sim_t, *sargs: str) -> Tuple[Optional[abstract_device_t], Optional[int]]:
        base = int(sargs[0], 16) if sargs else 0x8000_0000
        return MyDevice(sim), base

    # pylint: disable=unused-argument
    def generate_dts(self, sim: sim_t, *sargs: str) -> str:
        return "DTS"


class MyFactoryNoBase(device_factory_t):

    # pylint: disable=unused-argument
    def parse_from_fdt(self, fdt, sim: sim_t, *sargs: str) -> Tuple[Optional[abstract_device_t], Optional[int]]:
        return MyDevice(sim), None

    # pylint: disable=unused-argument
    def generate_dts(self, sim: sim_t, *sargs: str) -> str:
        return ""


def test_abstract_device_t(mock_sim):
    """
    instantiate an abstract device subclass and test its methods
    """
    dev = MyDevice(mock_sim)
    # call device methods from the C++ side
    assert _test_mmio_load(dev, 0, 13) == b"hello world!\n"
    assert _test_mmio_store(dev, 0, b"hello again!\n") is None


def test_device_factory_t(mock_sim):
    """
    instantiate a device factory subclass and test its methods
    """
    fact = MyFactory()
    dev, base = _test_mmio_parse_from_fdt(fact, None, mock_sim, "0x4000_0000")
    assert isinstance(dev, abstract_device_t)
    assert base == 0x4000_0000
    assert isinstance(dev.sim, sim_t)
    assert isinstance(dev.sim.plic, plic_t)


def test_device_factory_t_no_base(mock_sim):
    fact = MyFactoryNoBase()
    dev, base = _test_mmio_parse_from_fdt(fact, None, mock_sim)
    assert isinstance(dev, abstract_device_t)
    assert base is None
    assert isinstance(dev.sim, sim_t)
    assert isinstance(dev.sim.plic, plic_t)


@pytest.mark.parametrize("name,sargs,fact_cls,dev_cls", [
    pytest.param("my", ("0x8000_0000", ), MyFactory, MyDevice, id="my"),
])
def test_mmio_device_map(mock_sim, name, sargs, fact_cls, dev_cls):
    # register
    fact = MyFactory()
    mmio_device_map[name] = fact
    # retrieve
    assert name in mmio_device_map
    mmio_device_map[name] = fact
    mmio_device_map[name] = fact
    # type check
    assert isinstance(mmio_device_map[name], fact_cls)
    assert mmio_device_map[name] is fact
    # method_call: parse_from_fdt
    dev, base = mmio_device_map[name].parse_from_fdt(None, mock_sim, *sargs)
    assert isinstance(dev, dev_cls)
    assert base == 0x8000_0000
    # method_call: generate_dts
    assert mmio_device_map[name].generate_dts(mock_sim, *sargs) == "DTS"
    # unregister
    del mmio_device_map[name]
    assert name not in mmio_device_map
    del mmio_device_map[name]   # idempotent of deletion


@pytest.mark.parametrize("name", [
    "clint", "plic", "ns16550"
])
def test_mmio_device_map_builtin(name):
    assert name in mmio_device_map
    assert isinstance(mmio_device_map[name], device_factory_t)


@pytest.mark.parametrize("name,sargs,dts", [
    pytest.param("clint", (), """    clint@2000000 {
      compatible = "riscv,clint0";
      interrupts-extended = <&CPU0_intc 3 &CPU0_intc 7 >;
      reg = <0x0 0x2000000 0x0 0xc0000>;
    };
""", id="clint"),
    pytest.param("plic", (), """    PLIC: plic@c000000 {
      compatible = "riscv,plic0";
      #address-cells = <2>;
      interrupts-extended = <&CPU0_intc 11 &CPU0_intc 9 >;
      reg = <0x0 0xc000000 0x0 0x1000000>;
      riscv,ndev = <0x1f>;
      riscv,max-priority = <0xf>;
      #interrupt-cells = <1>;
      interrupt-controller;
    };
""", id="plic"),
    pytest.param("ns16550", (), """    SERIAL0: ns16550@10000000 {
      compatible = "ns16550a";
      clock-frequency = <10000000>;
      interrupt-parent = <&PLIC>;
      interrupts = <1>;
      reg = <0x0 0x10000000 0x0 0x100>;
      reg-shift = <0x0>;
      reg-io-width = <0x1>;
    };
""", id="ns16550"),
])
def test_device_factory_t_generate_dts(mock_sim, name, sargs, dts):
    assert name in mmio_device_map
    assert mmio_device_map[name].generate_dts(mock_sim, *sargs) == dts
