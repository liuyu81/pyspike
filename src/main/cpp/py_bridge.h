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
#include <pybind11/functional.h>
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
             std::is_base_of<disasm_insn_t,
                             typename std::remove_pointer<T>::type>::value ||
             std::is_base_of<insn_desc_t,
                             typename std::remove_pointer<T>::type>::value ||
             std::is_same<T, insn_func_t>::value),
        "T must be abstract_device_t, device_factory_t, processor_t, arg_t, "
        "csr_t, rocc_t, extension_t, insn_desc_t, insn_func_t, or disasm_insn_t");
    py_obj.inc_ref();
    if constexpr (std::is_same<T, insn_func_t>::value) {
      pybind11::function cast = pybind11::module_::import("ctypes").attr("cast");
      pybind11::function c_void_p = pybind11::module_::import("ctypes").attr("c_void_p");
      pybind11::function py2ct =
          pybind11::module_::import("riscv._riscv.processor").attr("insn_func_py2ct");
      // cast python callable to ctypes function pointer
      auto py_ct = py2ct(py_obj);
      py_ct.inc_ref();
      references.emplace(reinterpret_cast<uint64_t>(py_ct.ptr()), py_ct);
      // cast ctypes function pointer to void* then to T
      auto py_ct_void_p = cast(py_ct, c_void_p);
      auto obj = pybind11::cast<uint64_t>(py_ct_void_p.attr("value"));
      references.emplace(obj, py_obj);
      return reinterpret_cast<T>(obj);
    }
    T obj = pybind11::cast<T>(py_obj);
    references.emplace(reinterpret_cast<uint64_t>(obj), py_obj);
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
