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

namespace py = pybind11;

std::vector<insn_desc_t> py_extension_t::get_instructions() {
  std::vector<insn_desc_t> instructions;
  auto &bridge = PythonBridge::getInstance();
  try {
    py::function py_method = py::get_override(this, "get_instructions");
    py::sequence py_seq = py_method();
    for (const auto &py_obj : py_seq) {
      instructions.push_back(*bridge.track<insn_desc_t *>(py_obj));
    }
  } catch (py::error_already_set &e) {
    std::cerr << e.what() << std::endl;
  }
  return instructions;
}

std::vector<disasm_insn_t *> py_extension_t::get_disasms() {
  std::vector<disasm_insn_t *> disasms;
  auto &bridge = PythonBridge::getInstance();
  try {
    py::function py_method = py::get_override(this, "get_disasms");
    py::sequence py_seq = py_method();
    for (const auto &py_obj : py_seq) {
      disasms.push_back(bridge.track<disasm_insn_t *>(py_obj));
    }
  } catch (py::error_already_set &e) {
    std::cerr << e.what() << std::endl;
  }
  return disasms;
}

const char *py_extension_t::name() {
  PYBIND11_OVERRIDE_PURE_NAME(const char *, extension_t, "_name", name);
}

void py_extension_t::reset() {
  PYBIND11_OVERRIDE(void, extension_t, reset);
}

void py_extension_t::set_debug(bool value) {
  PYBIND11_OVERRIDE(void, extension_t, set_debug, value);
}

reg_t py_rocc_t::custom0(rocc_insn_t insn, reg_t xs1, reg_t xs2) {
  PYBIND11_OVERRIDE(reg_t, rocc_t, custom0, insn, xs1, xs2);
}

reg_t py_rocc_t::custom1(rocc_insn_t insn, reg_t xs1, reg_t xs2) {
  PYBIND11_OVERRIDE(reg_t, rocc_t, custom1, insn, xs1, xs2);
}

reg_t py_rocc_t::custom2(rocc_insn_t insn, reg_t xs1, reg_t xs2) {
  PYBIND11_OVERRIDE(reg_t, rocc_t, custom2, insn, xs1, xs2);
}

reg_t py_rocc_t::custom3(rocc_insn_t insn, reg_t xs1, reg_t xs2) {
  PYBIND11_OVERRIDE(reg_t, rocc_t, custom3, insn, xs1, xs2);
}

const char *py_rocc_t::name() {
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
