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
#ifndef _RISCV_DISASM_H_
#define _RISCV_DISASM_H_

#include <string>

#include <riscv/disasm.h>

#include <pybind11/pybind11.h>

class py_arg_t : public arg_t {
public:
  virtual std::string to_string(insn_t val) const override;
};

// py signature: disasm_insn_t(name: str, match: int, mask: int, *args: arg_t)
// -> disasm_insn_t
disasm_insn_t *py_disasm_insn_t_create(const std::string &name, uint32_t match,
                                       uint32_t mask, pybind11::args args);

#endif // _RISCV_DISASM_H_
