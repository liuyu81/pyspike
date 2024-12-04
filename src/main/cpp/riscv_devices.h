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
#ifndef _RISCV_DEVICE_H_
#define _RISCV_DEVICE_H_

#include <riscv/abstract_device.h>
#include <riscv/sim.h>

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "py_bridge.h"

// trampoline helper class for extending abstract_device_t
class py_abstract_device_t : public abstract_device_t {
public:
  using abstract_device_t::abstract_device_t;

public:
  // py signature: `(addr: int, size: int) -> bytes`
  virtual bool load(reg_t addr, size_t len, uint8_t *bytes) override;
  // py signature: `(addr: int, data: bytes) -> None`
  virtual bool store(reg_t addr, size_t len, const uint8_t *bytes) override;
  // py signature: `(rtc_ticks: int) -> None`
  virtual void tick(reg_t rtc_ticks) override;
};

// trampoline helper class for extending device_factory_t
class py_device_factory_t : public device_factory_t {
public:
  using device_factory_t::device_factory_t;
  py_device_factory_t();

public:
  // py signature: `(fdt, sim, *sargs) -> (abstract_device_t, int | None)`
  virtual abstract_device_t *
  parse_from_fdt(const void *fdt, const sim_t *sim, reg_t *base,
                 const std::vector<std::string> &sargs) const override;
  // py signature: `(sim, *sargs) -> str`
  virtual std::string
  generate_dts(const sim_t *sim,
               const std::vector<std::string> &sargs) const override;
};

// helper class for accessing mmio_device_map
class py_mmio_factory_map_t {
public:
  py_mmio_factory_map_t();

public:
  size_t len() const;
  bool contains(const std::string &name) const;
  const device_factory_t *getitem(const std::string &name) const;
  void setitem(const std::string &name, pybind11::handle py_mmio_fact);
  void delitem(const std::string &name);
  pybind11::iterator iter() const;
  pybind11::str repr() const;
};

// helper for Python -> C++ -> Python calls to `abstract_device_t::load`
pybind11::bytes py_mmio_load(abstract_device_t &dev, reg_t addr, size_t len);

// helper for Python -> C++ -> Python calls to `abstract_device_t::store`
void py_mmio_store(abstract_device_t &dev, reg_t addr, pybind11::bytes data);

// helper for Python -> C++ -> Python calls to `abstract_device_t::tick`
void py_mmio_tick(abstract_device_t &dev, reg_t rtc_ticks);

// helper for Python -> C++ -> Python calls to
// `device_factory_t::parse_from_fdt`
pybind11::tuple py_mmio_parse_from_fdt(const device_factory_t &fact,
                                       const void *fdt, const sim_t *sim,
                                       pybind11::args sargs);

// helper for Python -> C++ -> Python calls to `device_factory_t::generate_dts`
pybind11::str py_mmio_generate_dts(const device_factory_t &fact,
                                   const sim_t *sim, pybind11::args sargs);

#endif // _RISCV_DEVICE_H_
