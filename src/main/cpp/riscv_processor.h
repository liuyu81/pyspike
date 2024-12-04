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
#ifndef _RISCV_PROCESSOR_H_
#define _RISCV_PROCESSOR_H_

#include <functional>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <riscv/decode.h>
#include <riscv/processor.h>

// proxy to state_t::log_reg_write
class py_commit_log_reg_t {
public:
  py_commit_log_reg_t(commit_log_reg_t& ref);

public:
  size_t len() const;
  bool contains(reg_t regnum) const;
  freg_t getitem(reg_t regnum) const;
  void setitem(reg_t regnum, freg_t data);
  void setitem(reg_t regnum, pybind11::tuple data);
  void delitem(reg_t regnum);
  void clear();
  pybind11::str repr() const;

private:
  commit_log_reg_t& ref;
};


// proxy to state_t::log_mem_{read,write}
class py_commit_log_mem_t {
public:
  py_commit_log_mem_t(commit_log_mem_t& ref);

public:
  size_t len() const;
  void append(const std::tuple<reg_t, uint64_t, uint8_t>& item);
  std::tuple<reg_t, uint64_t, uint8_t> getitem(int index);
  void clear();
  pybind11::str repr() const;

private:
  commit_log_mem_t& ref;
};


// py signature : insn_desc_t_create(
//     match: int,
//     mask: int,
//     fast_rv32i: Callable[[processor_t, insn_t, int], int],
//     fast_rv64i: Callable[[processor_t, insn_t, int], int],
//     fast_rv32e: Callable[[processor_t, insn_t, int], int],
//     fast_rv64e: Callable[[processor_t, insn_t, int], int],
//     logged_rv32i: Callable[[processor_t, insn_t, int], int],
//     logged_rv64i: Callable[[processor_t, insn_t, int], int],
//     logged_rv32e: Callable[[processor_t, insn_t, int], int],
//     logged_rv64e: Callable[[processor_t, insn_t, int], int]
// ) -> insn_desc_t
insn_desc_t *py_insn_desc_t_create(
    insn_bits_t match, insn_bits_t mask, pybind11::function fast_rv32i,
    pybind11::function fast_rv64i, pybind11::function fast_rv32e,
    pybind11::function fast_rv64e, pybind11::function logged_rv32i,
    pybind11::function logged_rv64i, pybind11::function logged_rv32e,
    pybind11::function logged_rv64e);

#endif // _RISCV_PROCESSOR_H_
