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
#ifndef _RISCV_SIM_H_
#define _RISCV_SIM_H_

#include <map>
#include <vector>

#include <riscv/devices.h>
#include <riscv/processor.h>
#include <riscv/sim.h>

#include "riscv_cfg.h"

// trampoline helper class for extending sim_t
class py_sim_t : public sim_t, pybind11::trampoline_self_life_support {
public:
  using sim_t::sim_t;

public:
  virtual void proc_reset(unsigned id) override;

public:
  static py_sim_t *
  create(const managed_cfg_t &cfg, bool halted,
         const std::vector<std::pair<std::string, std::vector<std::string>>>
             &plugin_device_factories,
         const std::vector<std::string> &args,
         const debug_module_config_t &dm_config,
         const std::optional<std::string>& log_path,
         bool dtb_enabled,
         const std::optional<std::string>& dtb_file,
         bool socket_enabled,
         const std::optional<FILE *>& cmd_file,
         std::optional<unsigned long long> instruction_limit);
};

#endif // _RISCV_SIM_H_
