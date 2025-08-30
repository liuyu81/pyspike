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
#ifndef _PYTHON_BRIDGE_H_
#define _PYTHON_BRIDGE_H_

#include <functional>
#include <mutex>

#include <riscv/abstract_device.h>
#include <riscv/csrs.h>
#include <riscv/disasm.h>
#include <riscv/extension.h>
#include <riscv/processor.h>
#include <riscv/rocc.h>

#include <pybind11/embed.h>
#include <pybind11/stl.h>

class PythonBridge {
private:
  PythonBridge();
  ~PythonBridge();

private:
  PythonBridge(const PythonBridge &) = delete;
  PythonBridge(const PythonBridge &&) = delete;
  PythonBridge &operator=(const PythonBridge &) = delete;
  PythonBridge &operator=(const PythonBridge &&) = delete;

public:
  // returns singleton instance of python bridge
  static PythonBridge &getInstance();

public:
  // keep the python object alive on the C++ side
  template <typename T> T track(pybind11::handle py_obj) {
    static_assert(
        std::is_pointer<T>::value &&
            (std::is_base_of<abstract_device_t,
                             typename std::remove_pointer<T>::type>::value ||
             std::is_base_of<const device_factory_t,
                             typename std::remove_pointer<T>::type>::value ||
             std::is_base_of<processor_t,
                             typename std::remove_pointer<T>::type>::value ||
             std::is_base_of<arg_t,
                             typename std::remove_pointer<T>::type>::value ||
             std::is_base_of<csr_t,
                             typename std::remove_pointer<T>::type>::value ||
             std::is_base_of<rocc_t,
                             typename std::remove_pointer<T>::type>::value ||
             std::is_base_of<extension_t,
                             typename std::remove_pointer<T>::type>::value ||
             std::is_base_of<insn_desc_t,
                             typename std::remove_pointer<T>::type>::value ||
             std::is_base_of<disasm_insn_t,
                             typename std::remove_pointer<T>::type>::value),
        "T must be abstract_device_t, device_factory_t, processor_t, arg_t, "
        "csr_t, rocc_t, extension_t, insn_desc_t, or disasm_insn_t");
    T obj = pybind11::cast<T>(py_obj);
    uint64_t addr = reinterpret_cast<uint64_t>(obj);
    if (references.emplace(addr, py_obj).second) {
      py_obj.inc_ref();
    }
    return obj;
  };

private:
  // do we need to initialize the python interpreter?
  bool standalone;

  // references to python objects that need to be kept alive
  std::map<uint64_t, pybind11::handle> references;

private:
  static PythonBridge singleton;
};

std::string format_ptr(const void *ptr, size_t width = 16);

std::ostream &operator<<(std::ostream &os, insn_func_t &f);

#endif // _PYTHON_BRIDGE_H_
