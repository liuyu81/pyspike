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
#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "py_bridge.h"

namespace py = pybind11;

PythonBridge::PythonBridge() : standalone(!Py_IsInitialized()), references() {
  if (standalone) {
    py::initialize_interpreter();
  }
  // load Python (IP) libraries
  py::exec(R"(
    import importlib
    import os
    import pathlib
    import sys
    if (pylibs := os.environ.get(")" ENV_PYSPIKE_LIBS R"(")):
        for mod_path in map(pathlib.Path, pylibs.split(os.pathsep)):
            sys.path.insert(0, mod_path.parent.as_posix())
            if mod_path.suffix in (".py", ".pyc", ".pyd"):
                mod_name = mod_path.with_suffix("").name
            else:
                mod_name = mod_path.name
            importlib.import_module(mod_name)
  )");
}

PythonBridge &PythonBridge::getInstance() {
  return PythonBridge::singleton;
}

PythonBridge::~PythonBridge() {
  references.clear();
  if (standalone) {
    py::finalize_interpreter();
  }
}

std::string format_ptr(const void *ptr, size_t width) {
  std::string result;
  result.reserve(width + 2);
  result.append("0x");
  uint64_t value = reinterpret_cast<uint64_t>(ptr);
  for (size_t i = 0; i < width; i++) {
    result.push_back("0123456789abcdef"[(value >> (width - i - 1) * 4) & 0xf]);
  }
  return result;
}

std::ostream &operator<<(std::ostream &os, insn_func_t &f) {
  os << std::hex << reinterpret_cast<void *>(f);
  return os;
}
