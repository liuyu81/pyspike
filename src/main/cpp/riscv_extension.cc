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
#include "riscv_extension.h"
#include "py_bridge.h"
#include "riscv_csrs.h"

namespace py = pybind11;

std::vector<insn_desc_t>
py_extension_t::get_instructions(const processor_t &proc) {
  std::vector<insn_desc_t> instructions;
  auto &bridge = PythonBridge::getInstance();
  try {
    py::function py_method = py::get_override(this, "get_instructions");
    py::object py_proc = py::cast(&proc);
    py::sequence py_seq = py_method(*bridge.track<processor_t *>(py_proc));
    for (const auto &py_obj : py_seq) {
      instructions.push_back(*bridge.track<insn_desc_t *>(py_obj));
    }
  } catch (py::error_already_set &e) {
    std::cerr << e.what() << std::endl;
  }
  return instructions;
}

std::vector<disasm_insn_t *>
py_extension_t::get_disasms(const processor_t *proc) {
  std::vector<disasm_insn_t *> disasms;
  auto &bridge = PythonBridge::getInstance();
  try {
    py::function py_method = py::get_override(this, "get_disasms");
    py::object py_proc = py::cast(proc);
    py::sequence py_seq = py_method(*bridge.track<processor_t *>(py_proc));
    for (const auto &py_obj : py_seq) {
      disasms.push_back(bridge.track<disasm_insn_t *>(py_obj));
    }
  } catch (py::error_already_set &e) {
    std::cerr << e.what() << std::endl;
  }
  return disasms;
}

std::vector<csr_t_p> py_extension_t::get_csrs(processor_t &proc) const {
  std::vector<csr_t_p> csrs;
  auto &bridge = PythonBridge::getInstance();
  try {
    py::function py_method = py::get_override(this, "get_csrs");
    py::object py_proc = py::cast(&proc);
    py::sequence py_seq = py_method(*bridge.track<processor_t *>(py_proc));
    for (const auto &py_obj : py_seq) {
      csrs.push_back(bridge.track<py_csr_t *>(py_obj)->shared_from_this());
    }
  } catch (py::error_already_set &e) {
    std::cerr << e.what() << std::endl;
  }
  return csrs;
}

const char *py_extension_t::name() const {
  PYBIND11_OVERRIDE_PURE_NAME(const char *, extension_t, "_name", name);
}

void py_extension_t::reset(processor_t &proc) {
  auto &bridge = PythonBridge::getInstance();
  auto py_proc = py::cast(&proc);
  PYBIND11_OVERRIDE(void, extension_t, reset,
                    *bridge.track<processor_t *>(py_proc));
}

void py_extension_t::set_debug(bool value, const processor_t &proc) {
  auto &bridge = PythonBridge::getInstance();
  auto py_proc = py::cast(&proc);
  PYBIND11_OVERRIDE(void, extension_t, set_debug, value,
                    *bridge.track<processor_t *>(py_proc));
}

reg_t py_rocc_t::custom0(processor_t *proc, rocc_insn_t insn, reg_t xs1,
                         reg_t xs2) {
  PYBIND11_OVERRIDE(reg_t, rocc_t, custom0, proc, insn, xs1, xs2);
}

reg_t py_rocc_t::custom1(processor_t *proc, rocc_insn_t insn, reg_t xs1,
                         reg_t xs2) {
  PYBIND11_OVERRIDE(reg_t, rocc_t, custom1, proc, insn, xs1, xs2);
}

reg_t py_rocc_t::custom2(processor_t *proc, rocc_insn_t insn, reg_t xs1,
                         reg_t xs2) {
  PYBIND11_OVERRIDE(reg_t, rocc_t, custom2, proc, insn, xs1, xs2);
}

reg_t py_rocc_t::custom3(processor_t *proc, rocc_insn_t insn, reg_t xs1,
                         reg_t xs2) {
  PYBIND11_OVERRIDE(reg_t, rocc_t, custom3, proc, insn, xs1, xs2);
}

const char *py_rocc_t::name() const {
  PYBIND11_OVERRIDE_PURE_NAME(const char *, rocc_t, "_name", name);
}

void py_register_extension(const std::string &name, py::function py_ctor) {
  register_extension(name.c_str(), [py_ctor]() -> extension_t * {
    auto py_ext = py_ctor();
    if (py::isinstance<rocc_t>(py_ext)) {
      return PythonBridge::getInstance().track<rocc_t *>(py_ext);
    }
    return PythonBridge::getInstance().track<extension_t *>(py_ext);
  });
}
