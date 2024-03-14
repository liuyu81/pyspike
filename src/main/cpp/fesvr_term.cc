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
#include <fesvr/term.h>

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "fesvr_term.h"

namespace py = pybind11;

py_canonical_terminal_t::py_canonical_terminal_t() {
  // NOP
}

py_canonical_terminal_t::~py_canonical_terminal_t() {
  // NOP
}

py::bytes py_canonical_terminal_t::read() {
  std::vector<char> data;
  int ch = canonical_terminal_t::read();
  while (ch != -1) {
    data.push_back(static_cast<char>(ch));
    ch = canonical_terminal_t::read();
  }
  return py::bytes(data.data(), data.size());
}

void py_canonical_terminal_t::write(const py::bytes &data) {
  std::string str(data);
  for (char c : str) {
    canonical_terminal_t::write(c);
  }
}
