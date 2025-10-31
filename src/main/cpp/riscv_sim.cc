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
#include <map>
#include <optional>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "riscv_sim.h"

void py_sim_t::proc_reset(unsigned id) {
  PYBIND11_OVERRIDE(void, sim_t, proc_reset, id);
}

py_sim_t *py_sim_t::create(
    const managed_cfg_t &cfg, bool halted,
    const std::vector<std::pair<std::string, std::vector<std::string>>>
        &plugin_device_factories,
    const std::vector<std::string> &args,
    const debug_module_config_t &dm_config) {
  // allocate mem based on mem_layout
  std::vector<std::pair<reg_t, abstract_mem_t *>> mems;
  mems.reserve(cfg.mem_layout.size());
  for (const auto &cfg : cfg.mem_layout) {
    mems.push_back(std::make_pair(cfg.get_base(), new mem_t(cfg.get_size())));
  }
  // lookup device factories
  const mmio_device_map_t &registry = mmio_device_map();
  std::vector<device_factory_sargs_t> factories;
  for (const auto &[k, v] : plugin_device_factories) {
    const device_factory_t *factory = registry.at(k);
    const std::vector<std::string> &sargs = v;
    factories.push_back(std::make_pair(factory, sargs));
  }
  // allocate py_sim_t instance
  return new py_sim_t(&cfg, halted, mems, factories, args, dm_config, nullptr,
                      true, nullptr, false, nullptr, std::nullopt);
}
