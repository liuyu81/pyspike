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

#include <pybind11/embed.h>

#include <riscv/processor.h>
#include <riscv/trap.h>

#include "riscv_csrs.h"

namespace py = pybind11;

py_csr_t::py_csr_t(processor_t *const proc, const reg_t addr)
    : csr_t(proc, addr), keepalive(this) {
  // NOP
}

void py_csr_t::verify_permissions(insn_t insn, bool write) const {
  PYBIND11_OVERRIDE(void, csr_t, verify_permissions, insn, write);
}

reg_t py_csr_t::read() const noexcept {
  PYBIND11_OVERRIDE_PURE(reg_t, csr_t, read);
}

bool py_csr_t::unlogged_write(const reg_t val) noexcept {
  PYBIND11_OVERRIDE_PURE(bool, csr_t, unlogged_write, val);
}

reg_t py_csr_t::written_value() const noexcept {
  PYBIND11_OVERRIDE(reg_t, csr_t, written_value);
}
