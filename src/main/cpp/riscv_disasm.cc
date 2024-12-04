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
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "py_bridge.h"
#include "riscv_disasm.h"

namespace py = pybind11;

std::string py_arg_t::to_string(insn_t insn) const {
  PYBIND11_OVERLOAD_PURE(std::string, arg_t, to_string, insn);
}

disasm_insn_t *py_disasm_insn_t_create(const std::string &name, uint32_t match,
                                       uint32_t mask, py::args py_args) {
  std::vector<const arg_t *> raw_args;
  auto &bridge = PythonBridge::getInstance();
  for (auto py_obj : py_args) {
    raw_args.push_back(bridge.track<const arg_t *>(py_obj));
  }
  return new disasm_insn_t(name.c_str(), match, mask, raw_args);
}
