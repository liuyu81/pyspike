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
#ifndef _RISCV_EXTENSION_H_
#define _RISCV_EXTENSION_H_

#include <functional>
#include <vector>

#include <riscv/extension.h>
#include <riscv/processor.h>
#include <riscv/rocc.h>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// trampoline helper class for extending extension_t
class py_extension_t : public extension_t,
                       public pybind11::trampoline_self_life_support {
public:
  using extension_t::extension_t;

public:
  virtual std::vector<insn_desc_t>
  get_instructions(const processor_t &proc) override;
  virtual std::vector<disasm_insn_t *>
  get_disasms(const processor_t *proc = nullptr) override;
  virtual std::vector<csr_t_p> get_csrs(processor_t &proc) const override;

public:
  virtual const char *name() const override;
  virtual void reset(processor_t &proc) override;
  virtual void set_debug(bool UNUSED value, const processor_t &proc) override;

public:
  // make protected members accessible from python
  using extension_t::clear_interrupt;
  using extension_t::illegal_instruction;
  using extension_t::raise_interrupt;
};

// trampoline helper class for extending rocc_t
class py_rocc_t : public rocc_t, public pybind11::trampoline_self_life_support {
public:
  using rocc_t::rocc_t;

public:
  virtual reg_t custom0(processor_t *proc, rocc_insn_t insn, reg_t xs1,
                        reg_t xs2) override;
  virtual reg_t custom1(processor_t *proc, rocc_insn_t insn, reg_t xs1,
                        reg_t xs2) override;
  virtual reg_t custom2(processor_t *proc, rocc_insn_t insn, reg_t xs1,
                        reg_t xs2) override;
  virtual reg_t custom3(processor_t *proc, rocc_insn_t insn, reg_t xs1,
                        reg_t xs2) override;

public:
  virtual const char *name() const override;
};

// helper for Python -> C++ -> Python calls to `register_extension`
void py_register_extension(const std::string &name, pybind11::function py_ctor);

#endif // _RISCV_EXTENSION_H_
