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
#include "riscv_decode.h"

namespace py = pybind11;

insn_bits_t insn_fetch_one(py::bytes data) {
  auto view = data.attr("__getitem__");
  auto from_bytes = py::int_().attr("from_bytes");
  int length = insn_length(view(0).cast<unsigned char>());
  auto insn = from_bytes(view(py::slice(0, length, 1)), "little");
  return insn.cast<insn_bits_t>();
}

std::vector<insn_bits_t> insn_fetch_all(py::bytes data) {
  std::vector<insn_bits_t> sequence;
  auto view = data.attr("__getitem__");
  auto from_bytes = py::int_().attr("from_bytes");
  auto total = py::len(data);
  size_t offset = 0;
  while (offset < total) {
    int length = insn_length(view(offset).cast<unsigned char>());
    if (offset + length > total) {
      break;
    }
    auto insn =
        from_bytes(view(py::slice(offset, offset + length, 1)), "little");
    sequence.push_back(insn.cast<insn_bits_t>());
    offset += length;
  }
  return sequence;
}
