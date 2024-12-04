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
#ifndef _RISCV_CSRS_H_
#define _RISCV_CSRS_H_

#include <memory>

#include <riscv/csrs.h>

class py_csr_t : public csr_t, public std::enable_shared_from_this<csr_t> {
public:
  py_csr_t(processor_t *const proc, const reg_t addr);

public:
  virtual void verify_permissions(insn_t insn, bool write) const override;
  virtual reg_t read() const noexcept override;

public:
  // make protected methods accessible from python
  virtual bool unlogged_write(const reg_t val) noexcept override;
  virtual reg_t written_value() const noexcept override;
  using csr_t::proc;
  using csr_t::state;

private:
  // keep myself alive till after the python object is garbage-collected
  csr_t_p keepalive;
};

#endif // _RISCV_CSRS_H_
