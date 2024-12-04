/*
 * Copyright 2024 WuXi EsionTech Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <iostream>

#include "riscv_devices.h"

#include "py_bridge.h"

namespace py = pybind11;

bool py_abstract_device_t::load(reg_t addr, size_t len, uint8_t *bytes) {
  try {
    py::function py_method = py::get_override(this, "load");
    py::bytes py_result = py_method(addr, len);
    py::buffer_info buf = py::cast<py::buffer>(py_result).request();
    if ((buf.ndim != 1) || (static_cast<size_t>(buf.shape[0]) != len)) {
      return false;
    }
    std::memcpy(bytes, buf.ptr, len);
    return true;
  } catch (py::error_already_set &e) {
    std::cerr << e.what() << std::endl;
  }
  return false;
}

bool py_abstract_device_t::store(reg_t addr, size_t len, const uint8_t *bytes) {
  try {
    py::function py_method = py::get_override(this, "store");
    py_method(addr, py::bytes(reinterpret_cast<const char *>(bytes), len));
    return true;
  } catch (py::error_already_set &e) {
    std::cerr << e.what() << std::endl;
  }
  return false;
}

void py_abstract_device_t::tick(reg_t rtc_ticks) {
  PYBIND11_OVERRIDE_PURE(void, abstract_device_t, tick, rtc_ticks);
}

py_device_factory_t::py_device_factory_t() {
  // NOP
}

abstract_device_t *py_device_factory_t::parse_from_fdt(
    const void *fdt, const sim_t *sim, reg_t *base,
    const std::vector<std::string> &sargs) const {
  try {
    py::function py_method = py::get_override(this, "parse_from_fdt");
    py::args py_sargs = py::cast(sargs);
    py::object py_result = py_method(fdt, sim, *py_sargs);
    py::handle py_dev;
    if (py::isinstance<py::tuple>(py_result)) {
      // if py_method returns a tuple, assume it's `(abstract_device_t, int)`
      py::tuple py_tuple_result = py_result.cast<py::tuple>();
      if ((base != nullptr) && (py_tuple_result.size() > 1)) {
        *base = py_tuple_result[1].cast<reg_t>();
      }
      py_dev = py_tuple_result[0];
    } else {
      // otherwise, assume it's just `abstract_device_t`
      py_dev = py_result;
    }
    return PythonBridge::getInstance().track<abstract_device_t *>(py_dev);
  } catch (py::error_already_set &e) {
    std::cerr << e.what() << std::endl;
  }
  return nullptr;
}

std::string
py_device_factory_t::generate_dts(const sim_t *sim,
                                  const std::vector<std::string> &sargs) const {
  try {
    py::function py_method = py::get_override(this, "generate_dts");
    py::args py_sargs = py::cast(sargs);
    py::handle py_dts = py_method(sim, *py_sargs);
    return py_dts.cast<std::string>();
  } catch (py::error_already_set &e) {
    std::cerr << e.what() << std::endl;
  }
  return "";
}

py_mmio_factory_map_t::py_mmio_factory_map_t() {
  // NOP
}

size_t py_mmio_factory_map_t::len() const {
  return mmio_device_map().size();
}

bool py_mmio_factory_map_t::contains(const std::string &name) const {
  return mmio_device_map().count(name) > 0;
}

const device_factory_t *
py_mmio_factory_map_t::getitem(const std::string &name) const {
  return mmio_device_map().at(name);
}

void py_mmio_factory_map_t::setitem(const std::string &name,
                                    pybind11::handle py_mmio_fact) {
  mmio_device_map()[name] =
      PythonBridge::getInstance().track<const device_factory_t *>(py_mmio_fact);
}

void py_mmio_factory_map_t::delitem(const std::string &name) {
  mmio_device_map().erase(name);
}

py::iterator py_mmio_factory_map_t::iter() const {
  return py::make_key_iterator(mmio_device_map().begin(),
                               mmio_device_map().end());
}

py::str py_mmio_factory_map_t::repr() const {
  std::ostringstream oss;
  bool first = true;
  oss << "{";
  for (const auto &[k, v] : mmio_device_map()) {
    if (!first) {
      oss << ", ";
    } else {
      first = false;
    }
    oss << "\"" << k << "\": "
        << "<riscv._riscv.devices.device_factory_t object at " << format_ptr(v)
        << ">";
  }
  oss << "}";
  return oss.str();
}

py::bytes py_mmio_load(abstract_device_t &device, reg_t addr, size_t len) {
  py::function py_method = py::get_override(&device, "load");
  if (py_method) {
    return py_method(addr, len);
  }
  std::string data(len, '\0');
  if (!device.load(addr, len, reinterpret_cast<uint8_t *>(data.data()))) {
    throw std::runtime_error("load failed");
  }
  return py::bytes(data);
}

void py_mmio_store(abstract_device_t &device, reg_t addr, py::bytes data) {
  py::function py_method = py::get_override(&device, "store");
  if (py_method) {
    py_method(addr, data);
    return;
  }
  py::buffer_info view = py::cast<py::buffer>(data).request();
  if (!device.store(addr, view.shape[0],
                    reinterpret_cast<const uint8_t *>(view.ptr))) {
    throw std::runtime_error("store failed");
  }
}

void py_mmio_tick(abstract_device_t &device, reg_t rtc_ticks) {
  py::function py_method = py::get_override(&device, "tick");
  if (py_method) {
    py_method(rtc_ticks);
    return;
  }
  device.tick(rtc_ticks);
}

py::tuple py_mmio_parse_from_fdt(const device_factory_t &factory,
                                 const void *fdt, const sim_t *sim,
                                 pybind11::args sargs) {
  py::function py_method = py::get_override(&factory, "parse_from_fdt");
  if (py_method) {
    return py_method(fdt, sim, *sargs);
  }
  reg_t base = 0;
  abstract_device_t *dev = factory.parse_from_fdt(
      fdt, sim, &base, sargs.cast<std::vector<std::string>>());
  return py::make_tuple(py::cast(dev), py::int_(base));
}

py::str py_mmio_generate_dts(const device_factory_t &factory, const sim_t *sim,
                             pybind11::args sargs) {
  py::function py_method = py::get_override(&factory, "generate_dts");
  if (py_method) {
    return py_method(sim, *sargs);
  }
  return factory.generate_dts(sim, sargs.cast<std::vector<std::string>>());
}
