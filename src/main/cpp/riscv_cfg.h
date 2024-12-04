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
#ifndef _RISCV_CFG_H_
#define _RISCV_CFG_H_

#include <optional>
#include <string>

#include <riscv/cfg.h>

// helper class for extending `cfg_t`
//
// `cfg_t` contains `const char *` properties pointing to externally allocated
// memories (`argv` or macro constants). with the python binding, we need to
// assign to these properties from the python side, and allow the r-value python
// variables to vanish after the assignment, without affecting `cfg_t`'s state.
// to achieve this, this `managed_cfg_t` allocates internal `std::string`'s for
// all externally allocated properties, and synchronizes the internal memory
// with the external memory both on read (get) and on write (set).
//
class managed_cfg_t : public cfg_t {
public:
  using cfg_t::cfg_t;
  managed_cfg_t();
  ~managed_cfg_t();

public:
  // copy-on-read getters of properties
  std::string get_bootargs();
  std::string get_isa();
  std::string get_priv();

  // setters of properties
  void set_bootargs(const std::string &bootargs);
  void set_isa(const std::string &isa);
  void set_priv(const std::string &priv);

public:
  static managed_cfg_t *create(std::optional<std::string> isa,
                               std::optional<std::string> priv,
                               std::optional<std::vector<mem_cfg_t>> mem_layout,
                               std::optional<reg_t> start_pc);

private:
  std::string _bootargs;
  std::string _isa;
  std::string _priv;
};

#endif // _RISCV_CFG_H_
