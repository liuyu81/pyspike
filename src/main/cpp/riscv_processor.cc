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

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "py_bridge.h"
#include "riscv_processor.h"

namespace py = pybind11;

py_commit_log_reg_t::py_commit_log_reg_t(commit_log_reg_t &ref) : ref(ref) {
  // NOP
}

size_t py_commit_log_reg_t::len() const {
  return this->ref.size();
}

bool py_commit_log_reg_t::contains(reg_t regnum) const {
  return this->ref.contains(regnum);
}

freg_t py_commit_log_reg_t::getitem(reg_t regnum) const {
  return this->ref[regnum];
}

void py_commit_log_reg_t::setitem(reg_t regnum, freg_t data) {
  this->ref[regnum] = data;
}

void py_commit_log_reg_t::setitem(reg_t regnum, py::tuple data) {
  freg_t packed_data = {py::cast<uint64_t>(data[0]),
                        py::cast<uint64_t>(data[1])};
  this->ref[regnum] = packed_data;
}

void py_commit_log_reg_t::delitem(reg_t regnum) {
  this->ref.erase(regnum);
}

void py_commit_log_reg_t::clear() {
  this->ref.clear();
}

py::str py_commit_log_reg_t::repr() const {
  std::ostringstream oss;
  oss << "{";
  oss << "}";
  return oss.str();
}

py_commit_log_mem_t::py_commit_log_mem_t(commit_log_mem_t &ref) : ref(ref) {
  // NOP
}

size_t py_commit_log_mem_t::len() const {
  return this->ref.size();
}

void py_commit_log_mem_t::append(
    const std::tuple<reg_t, uint64_t, uint8_t> &item) {
  this->ref.push_back(item);
}

std::tuple<reg_t, uint64_t, uint8_t> py_commit_log_mem_t::getitem(int index) {
  return this->ref[index];
}

void py_commit_log_mem_t::clear() {
  this->ref.clear();
}

py::str py_commit_log_mem_t::repr() const {
  std::ostringstream oss;
  oss << "{";
  oss << "}";
  return oss.str();
}

insn_desc_t *
py_insn_desc_t_create(insn_bits_t match, insn_bits_t mask,
                      py::function fast_rv32i, py::function fast_rv64i,
                      py::function fast_rv32e, py::function fast_rv64e,
                      py::function logged_rv32i, py::function logged_rv64i,
                      py::function logged_rv32e, py::function logged_rv64e) {
  py::function cast = py::module_::import("ctypes").attr("cast");
  py::function c_void_p = py::module_::import("ctypes").attr("c_void_p");
  py::function py2ct =
      py::module_::import("riscv._riscv.processor").attr("insn_func_py2ct");

  auto ct_fast_rv32i =
      py::cast<uint64_t>(cast(py2ct(fast_rv32i), c_void_p).attr("value"));
  auto ct_fast_rv64i =
      py::cast<uint64_t>(cast(py2ct(fast_rv64i), c_void_p).attr("value"));
  auto ct_fast_rv32e =
      py::cast<uint64_t>(cast(py2ct(fast_rv32e), c_void_p).attr("value"));
  auto ct_fast_rv64e =
      py::cast<uint64_t>(cast(py2ct(fast_rv64e), c_void_p).attr("value"));
  auto ct_logged_rv32i =
      py::cast<uint64_t>(cast(py2ct(logged_rv32i), c_void_p).attr("value"));
  auto ct_logged_rv64i =
      py::cast<uint64_t>(cast(py2ct(logged_rv64i), c_void_p).attr("value"));
  auto ct_logged_rv32e =
      py::cast<uint64_t>(cast(py2ct(logged_rv32e), c_void_p).attr("value"));
  auto ct_logged_rv64e =
      py::cast<uint64_t>(cast(py2ct(logged_rv64e), c_void_p).attr("value"));

  return new insn_desc_t{match,
                         mask,
                         reinterpret_cast<insn_func_t>(ct_fast_rv32i),
                         reinterpret_cast<insn_func_t>(ct_fast_rv64i),
                         reinterpret_cast<insn_func_t>(ct_fast_rv32e),
                         reinterpret_cast<insn_func_t>(ct_fast_rv64e),
                         reinterpret_cast<insn_func_t>(ct_logged_rv32i),
                         reinterpret_cast<insn_func_t>(ct_logged_rv64i),
                         reinterpret_cast<insn_func_t>(ct_logged_rv32e),
                         reinterpret_cast<insn_func_t>(ct_logged_rv64e)};
}
