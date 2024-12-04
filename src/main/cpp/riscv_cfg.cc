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
#include "riscv_cfg.h"

managed_cfg_t::managed_cfg_t() : cfg_t() {
  get_bootargs();
  get_isa();
  get_priv();
}

managed_cfg_t::~managed_cfg_t() {
  this->bootargs = nullptr;
  this->isa = nullptr;
  this->priv = nullptr;
}

std::string managed_cfg_t::get_bootargs() {
  if (!bootargs || !_bootargs.compare(bootargs)) {
    set_bootargs(bootargs ?: "");
  }
  return _bootargs;
}

void managed_cfg_t::set_bootargs(const std::string &bootargs) {
  _bootargs = bootargs;
  this->bootargs = _bootargs.c_str();
}

std::string managed_cfg_t::get_isa() {
  if (!isa || !_isa.compare(isa)) {
    set_isa(isa ?: "");
  }
  return _isa;
}

void managed_cfg_t::set_isa(const std::string &isa) {
  _isa = isa;
  this->isa = _isa.c_str();
}

std::string managed_cfg_t::get_priv() {
  if (!priv || !_priv.compare(priv)) {
    set_priv(priv ?: "");
  }
  return _priv;
}

void managed_cfg_t::set_priv(const std::string &priv) {
  _priv = priv;
  this->priv = _priv.c_str();
}

managed_cfg_t *
managed_cfg_t::create(std::optional<std::string> isa,
                      std::optional<std::string> priv,
                      std::optional<std::vector<mem_cfg_t>> mem_layout,
                      std::optional<reg_t> start_pc) {
  managed_cfg_t *cfg = new managed_cfg_t();
  if (isa.has_value()) {
    cfg->set_isa(isa.value());
  }
  if (priv.has_value()) {
    cfg->set_priv(priv.value());
  }
  if (mem_layout.has_value()) {
    cfg->mem_layout = mem_layout.value();
  }
  if (start_pc.has_value()) {
    cfg->start_pc = start_pc.value();
  }
  return cfg;
}
