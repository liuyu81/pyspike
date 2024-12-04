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
#include <dlfcn.h>

#include <memory>
#include <string>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <fesvr/term.h>
#include <riscv/decode.h>
#include <riscv/disasm.h>
#include <riscv/encoding.h>
#include <riscv/isa_parser.h>
#include <riscv/mmu.h>
#include <riscv/sim.h>

#include "fesvr_term.h"
#include "py_bridge.h"
#include "riscv_cfg.h"
#include "riscv_csrs.h"
#include "riscv_decode.h"
#include "riscv_devices.h"
#include "riscv_disasm.h"
#include "riscv_extension.h"
#include "riscv_processor.h"
#include "riscv_sim.h"

namespace py = pybind11;


PYBIND11_MODULE(_riscv, m) {

  m.doc() = "Python Bindings of Spike RISC-V ISA Simulator";

  // riscv.cfg
  {
    auto mod_cfg = m.def_submodule("cfg");

    py::class_<mem_cfg_t>(mod_cfg, "mem_cfg_t")
        .def(py::init<reg_t, reg_t>())
        .def_property_readonly("base", &mem_cfg_t::get_base)
        .def_property_readonly("size", &mem_cfg_t::get_size)
        .def_property_readonly("inclusive_end", &mem_cfg_t::get_inclusive_end);

    py::class_<managed_cfg_t>(mod_cfg, "cfg_t")
        .def(py::init())
        .def(py::init(&managed_cfg_t::create), py::kw_only(),
             py::arg("isa") = py::none(), py::arg("priv") = py::none(),
             py::arg("mem_layout") = py::none(),
             py::arg("start_pc") = py::none())
        .def_property("bootargs", &managed_cfg_t::get_bootargs,
                      &managed_cfg_t::set_bootargs)
        .def_property("isa", &managed_cfg_t::get_isa, &managed_cfg_t::set_isa)
        .def_property("priv", &managed_cfg_t::get_priv,
                      &managed_cfg_t::set_priv);
  }

  // riscv.csrs
  {
    auto mod_csrs = m.def_submodule("csrs");

    py::class_<csr_t, py_csr_t>(mod_csrs, "csr_t")
        .def(py::init([](processor_t *const proc, const reg_t addr) {
               return new py_csr_t(proc, addr);
             }),
             py::arg("proc"), py::arg("addr"))
        // csr_t members
        .def("verify_permissions", &csr_t::verify_permissions, py::arg("insn"),
             py::arg("write"))
        .def("read", &csr_t::read)
        .def_readonly("address", &csr_t::address)
        // csr_t protected members
        .def("unlogged_write", &py_csr_t::unlogged_write)
        .def("written_value", &py_csr_t::written_value)
        .def_readonly("proc", &py_csr_t::proc,
                      py::return_value_policy::reference_internal)
        .def_readonly("state", &py_csr_t::state,
                      py::return_value_policy::reference_internal);

    py::class_<basic_csr_t, csr_t>(mod_csrs, "basic_csr_t")
        .def(py::init<processor_t *const, const reg_t, const reg_t>(),
             py::arg("proc"), py::arg("addr"), py::arg("value"));

    py::class_<proxy_csr_t, csr_t>(mod_csrs, "proxy_csr_t")
        .def(py::init([](processor_t *const proc, const reg_t addr,
                         py::object py_csr) {
               auto raw_csr =
                   PythonBridge::getInstance().track<py_csr_t *>(py_csr);
               return new proxy_csr_t(proc, addr, raw_csr->shared_from_this());
             }),
             py::arg("proc"), py::arg("addr"), py::arg("delegate"));

    py::class_<const_csr_t, csr_t>(mod_csrs, "const_csr_t")
        .def(py::init<processor_t *const, const reg_t, reg_t>(),
             py::arg("proc"), py::arg("addr"), py::arg("val"));
  }

  // riscv.debug_module
  {
    auto mod_debug_module = m.def_submodule("debug_module");

    py::class_<debug_module_config_t>(mod_debug_module, "debug_module_config_t")
        .def(py::init())
        .def_readwrite("progbufsize", &debug_module_config_t::progbufsize)
        .def_readwrite("max_sba_data_width",
                       &debug_module_config_t::max_sba_data_width)
        .def_readwrite("require_authentication",
                       &debug_module_config_t::require_authentication)
        .def_readwrite("abstract_rti", &debug_module_config_t::abstract_rti)
        .def_readwrite("support_hasel", &debug_module_config_t::support_hasel)
        .def_readwrite("support_abstract_csr_access",
                       &debug_module_config_t::support_abstract_csr_access)
        .def_readwrite("support_abstract_fpr_access",
                       &debug_module_config_t::support_abstract_fpr_access)
        .def_readwrite("support_haltgroups",
                       &debug_module_config_t::support_haltgroups)
        .def_readwrite("support_impebreak",
                       &debug_module_config_t::support_impebreak);
  }

  // riscv.devices
  {
    auto mod_devices = m.def_submodule("devices");

    py::class_<abstract_device_t, py_abstract_device_t>(mod_devices,
                                                        "abstract_device_t")
        .def(py::init())
        .def("load", &py_mmio_load)
        .def("store", &py_mmio_store)
        .def("tick", &abstract_device_t::tick)
        .def("__repr__", [](const abstract_device_t &self) {
          return "<riscv._riscv.devices.abstract_device_t object at " +
                 format_ptr(&self) + ">";
        });

    py::class_<device_factory_t, py_device_factory_t>(mod_devices,
                                                      "device_factory_t")
        .def(py::init())
        .def("parse_from_fdt", &py_mmio_parse_from_fdt)
        .def("generate_dts", &py_mmio_generate_dts)
        .def("__repr__", [](const device_factory_t &self) {
          return "<riscv._riscv.devices.device_factory_t object at " +
                 format_ptr(&self) + ">";
        });

    py::class_<abstract_interrupt_controller_t>(
        mod_devices, "abstract_interrupt_controller_t")
        .def("set_interrupt_level",
             &abstract_interrupt_controller_t::set_interrupt_level);

    py::class_<bus_t, abstract_device_t>(mod_devices, "bus_t")
        .def("find_device", &bus_t::find_device);

    py::class_<rom_device_t, abstract_device_t>(mod_devices, "rom_device_t")
        .def(py::init<std::vector<char>>(), py::arg("data"));

    py::class_<abstract_mem_t, abstract_device_t>(mod_devices,
                                                  "abstract_mem_t");

    py::class_<mem_t, abstract_mem_t>(mod_devices, "mem_t")
        .def(py::init<reg_t>(), py::arg("size"));

    py::class_<clint_t, abstract_device_t>(mod_devices, "clint_t");

    py::class_<plic_t, abstract_device_t, abstract_interrupt_controller_t>(
        mod_devices, "plic_t", py::multiple_inheritance());

    py::class_<py_mmio_factory_map_t>(mod_devices, "mmio_factory_map_t")
        .def("__len__", &py_mmio_factory_map_t::len)
        .def("__contains__", &py_mmio_factory_map_t::contains)
        .def("__getitem__", &py_mmio_factory_map_t::getitem,
             py::return_value_policy::reference)
        .def("__setitem__", &py_mmio_factory_map_t::setitem)
        .def("__delitem__", &py_mmio_factory_map_t::delitem)
        .def("__iter__", &py_mmio_factory_map_t::iter)
        .def("__repr__", &py_mmio_factory_map_t::repr);

    mod_devices.attr("mmio_device_map") = new py_mmio_factory_map_t();
  }

  // riscv.decode
  auto mod_decode = m.def_submodule("decode");
  {
    mod_decode.attr("MAX_INSN_LENGTH") = MAX_INSN_LENGTH;
    mod_decode.attr("NXPR") = NXPR;
    mod_decode.attr("NFPR") = NFPR;
    mod_decode.attr("NVPR") = NVPR;
    mod_decode.attr("NCSR") = NCSR;

    // guess instruction length from the least significant byte (insn_bits_t
    // version)
    mod_decode.def(
        "insn_length",
        [](insn_bits_t bits) -> int { return insn_length(bits); },
        py::arg("bits"));

    // guess instruction length from the least significant byte (py::bytes
    // version)
    mod_decode.def(
        "insn_length",
        [](py::bytes data) -> int {
          auto view = data.attr("__getitem__");
          return insn_length(view(0).cast<unsigned char>());
        },
        py::arg("data"));

    // fetch a single instruction from a byte sequence
    mod_decode.def("insn_fetch_all", &insn_fetch_all, py::arg("data"));

    py::class_<insn_t>(mod_decode, "insn_t")
        .def(py::init<>())
        .def(py::init<insn_bits_t>(), py::arg("bits"))
        .def(py::init([](py::bytes bits) {
               return new insn_t(insn_fetch_one(bits));
             }),
             py::arg("bits"))
        .def_property_readonly("bits",
                               [](insn_t &self) {
                                 uint64_t bits = self.bits();
                                 return py::bytes(
                                     reinterpret_cast<const char *>(&bits),
                                     self.length());
                               })
        .def_property_readonly("i_imm", &insn_t::i_imm)
        .def_property_readonly("shamt", &insn_t::shamt)
        .def_property_readonly("s_imm", &insn_t::s_imm)
        .def_property_readonly("sb_imm", &insn_t::sb_imm)
        .def_property_readonly("u_imm", &insn_t::u_imm)
        .def_property_readonly("uj_imm", &insn_t::uj_imm)
        .def_property_readonly("rd", &insn_t::rd)
        .def_property_readonly("rs1", &insn_t::rs1)
        .def_property_readonly("rs2", &insn_t::rs2)
        .def_property_readonly("rs3", &insn_t::rs3)
        .def_property_readonly("rm", &insn_t::rm)
        .def_property_readonly("csr", &insn_t::csr)
        .def_property_readonly("iorw", &insn_t::iorw)
        .def_property_readonly("bs", &insn_t::bs)
        .def_property_readonly("rcon", &insn_t::rcon)

        .def_property_readonly("rvc_imm", &insn_t::rvc_imm)
        .def_property_readonly("rvc_zimm", &insn_t::rvc_zimm)
        .def_property_readonly("rvc_addi4spn_imm", &insn_t::rvc_addi4spn_imm)
        .def_property_readonly("rvc_addi16sp_imm", &insn_t::rvc_addi16sp_imm)
        .def_property_readonly("rvc_lwsp_imm", &insn_t::rvc_lwsp_imm)
        .def_property_readonly("rvc_ldsp_imm", &insn_t::rvc_ldsp_imm)
        .def_property_readonly("rvc_swsp_imm", &insn_t::rvc_swsp_imm)
        .def_property_readonly("rvc_sdsp_imm", &insn_t::rvc_sdsp_imm)
        .def_property_readonly("rvc_lw_imm", &insn_t::rvc_lw_imm)
        .def_property_readonly("rvc_ld_imm", &insn_t::rvc_ld_imm)
        .def_property_readonly("rvc_j_imm", &insn_t::rvc_j_imm)
        .def_property_readonly("rvc_b_imm", &insn_t::rvc_b_imm)
        .def_property_readonly("rvc_simm3", &insn_t::rvc_simm3)
        .def_property_readonly("rvc_rd", &insn_t::rvc_rd)
        .def_property_readonly("rvc_rs1", &insn_t::rvc_rs1)
        .def_property_readonly("rvc_rs2", &insn_t::rvc_rs2)
        .def_property_readonly("rvc_rs1s", &insn_t::rvc_rs1s)
        .def_property_readonly("rvc_rs2s", &insn_t::rvc_rs2s)

        .def_property_readonly("rvc_lbimm", &insn_t::rvc_lbimm)
        .def_property_readonly("rvc_lhimm", &insn_t::rvc_lhimm)

        .def_property_readonly("rvc_r1sc", &insn_t::rvc_r1sc)
        .def_property_readonly("rvc_r2sc", &insn_t::rvc_r2sc)
        .def_property_readonly("rvc_rlist", &insn_t::rvc_rlist)
        .def_property_readonly("rvc_spimm", &insn_t::rvc_spimm)

        .def_property_readonly("rvc_index", &insn_t::rvc_index)

        .def("__len__", &insn_t::length)
        .def("__eq__",
             [](insn_t &self, insn_t &other) {
               return self.bits() == other.bits();
             })
        .def("__eq__",
             [](insn_t &self, uint64_t other) { return self.bits() == other; })
        .def("__eq__", [](insn_t &self, py::bytes other) {
          uint64_t bits = self.bits();
          return py::bytes(reinterpret_cast<const char *>(&bits), self.length())
              .equal(other);
        });
  }

  // riscv.disasm
  {
    auto mod_disasm = m.def_submodule("disasm");

    py::class_<arg_t, py_arg_t>(mod_disasm, "arg_t")
        .def(py::init())
        .def("to_string", &arg_t::to_string, py::arg("insn"));

    py::class_<disasm_insn_t>(mod_disasm, "disasm_insn_t")
        .def(py::init(&py_disasm_insn_t_create), py::arg("name"),
             py::arg("match"), py::arg("mask"))
        .def_property_readonly("name", &disasm_insn_t::get_name)
        .def_property_readonly("match", &disasm_insn_t::get_match)
        .def_property_readonly("mask", &disasm_insn_t::get_mask)
        .def("to_string", &disasm_insn_t::to_string, py::arg("insn"))
        .def("__eq__", &disasm_insn_t::operator==);

    py::class_<disassembler_t>(mod_disasm, "disassembler_t")
        .def(py::init<const isa_parser_t *>(), py::arg("isa"))
        .def(
            "add_insn",
            [](disassembler_t &self, py::object py_insn) {
              self.add_insn(
                  PythonBridge::getInstance().track<disasm_insn_t *>(py_insn));
            },
            py::arg("insn"))
        .def("disassemble", &disassembler_t::disassemble)
        .def("lookup", &disassembler_t::lookup, py::return_value_policy::copy);

    mod_disasm.attr("xpr_name") =
        std::vector<std::string>(xpr_name, xpr_name + NXPR);
    mod_disasm.attr("fpr_name") =
        std::vector<std::string>(fpr_name, fpr_name + NFPR);
    mod_disasm.attr("vr_name") =
        std::vector<std::string>(vr_name, vr_name + NVPR);

    mod_disasm.def("csr_name", &csr_name);
  }

  // riscv.extension
  {
    auto mod_extension = m.def_submodule("extension");

    mod_extension.attr("ROCC_OPCODE0") = ROCC_OPCODE0;
    mod_extension.attr("ROCC_OPCODE1") = ROCC_OPCODE1;
    mod_extension.attr("ROCC_OPCODE2") = ROCC_OPCODE2;
    mod_extension.attr("ROCC_OPCODE3") = ROCC_OPCODE3;
    mod_extension.attr("ROCC_OPCODE_MASK") = ROCC_OPCODE_MASK;

    py::class_<rocc_insn_t>(mod_extension, "rocc_insn_t")
        .def_property_readonly(
            "opcode",
            [](const rocc_insn_t &self) -> unsigned { return self.opcode; })
        .def_property_readonly(
            "rd", [](const rocc_insn_t &self) -> unsigned { return self.rd; })
        .def_property_readonly(
            "xs2", [](const rocc_insn_t &self) -> unsigned { return self.xs2; })
        .def_property_readonly(
            "xs1", [](const rocc_insn_t &self) -> unsigned { return self.xs1; })
        .def_property_readonly(
            "xd", [](const rocc_insn_t &self) -> unsigned { return self.xd; })
        .def_property_readonly(
            "rs1", [](const rocc_insn_t &self) -> unsigned { return self.rs1; })
        .def_property_readonly(
            "rs2", [](const rocc_insn_t &self) -> unsigned { return self.rs2; })
        .def_property_readonly(
            "funct",
            [](const rocc_insn_t &self) -> unsigned { return self.funct; });

    py::class_<extension_t, py_extension_t>(mod_extension, "extension_t")
        .def(py::init())
        // extension_t members
        .def("get_instructions", &extension_t::get_instructions)
        .def("get_disasms", &extension_t::get_disasms)
        .def_property_readonly("name", &extension_t::name)
        .def("reset", &extension_t::reset)
        .def("set_debug", &extension_t::set_debug, py::arg("value"))
        .def("set_processor", &extension_t::set_processor, py::arg("_p"))
        // extension_t protected members
        .def_readonly("p", &py_extension_t::p,
                      py::return_value_policy::reference_internal)
        .def("illegal_instruction", &py_extension_t::illegal_instruction)
        .def("raise_interrupt", &py_extension_t::raise_interrupt)
        .def("clear_interrupt", &py_extension_t::clear_interrupt);

    py::class_<rocc_t, py_rocc_t, extension_t>(mod_extension, "rocc_t")
        .def(py::init())
        // rocc_t members
        .def("custom0", &rocc_t::custom0, py::arg("insn"), py::arg("xs1"),
             py::arg("xs2"))
        .def("custom1", &rocc_t::custom1, py::arg("insn"), py::arg("xs1"),
             py::arg("xs2"))
        .def("custom2", &rocc_t::custom2, py::arg("insn"), py::arg("xs1"),
             py::arg("xs2"))
        .def("custom3", &rocc_t::custom3, py::arg("insn"), py::arg("xs1"),
             py::arg("xs2"))
        .def("get_instructions", &rocc_t::get_instructions)
        .def("get_disasms", &rocc_t::get_disasms)
        .def_property_readonly("name", &rocc_t::name);

    mod_extension.def("find_extension", &find_extension, py::arg("name"))
        .def("register_extension", &py_register_extension, py::arg("name"),
             py::arg("f"), py::return_value_policy::reference);
  }

  // riscv.isa_parser
  {
    auto mod_isa_parser = m.def_submodule("isa_parser");

    py::enum_<isa_extension_t>(mod_isa_parser, "isa_extension_t")
        .value("zfh", isa_extension_t::EXT_ZFH)
        .value("zfhmin", isa_extension_t::EXT_ZFHMIN)
        .value("zba", isa_extension_t::EXT_ZBA)
        .value("zbb", isa_extension_t::EXT_ZBB)
        .value("zbc", isa_extension_t::EXT_ZBC)
        .value("zbs", isa_extension_t::EXT_ZBS)
        .value("zbkb", isa_extension_t::EXT_ZBKB)
        .value("zbkc", isa_extension_t::EXT_ZBKC)
        .value("zbkx", isa_extension_t::EXT_ZBKX)
        .value("zca", isa_extension_t::EXT_ZCA)
        .value("zcb", isa_extension_t::EXT_ZCB)
        .value("zcd", isa_extension_t::EXT_ZCD)
        .value("zcf", isa_extension_t::EXT_ZCF)
        .value("zcmlsd", isa_extension_t::EXT_ZCMLSD)
        .value("zcmp", isa_extension_t::EXT_ZCMP)
        .value("zcmt", isa_extension_t::EXT_ZCMT)
        .value("zknd", isa_extension_t::EXT_ZKND)
        .value("zkne", isa_extension_t::EXT_ZKNE)
        .value("zknh", isa_extension_t::EXT_ZKNH)
        .value("zksed", isa_extension_t::EXT_ZKSED)
        .value("zksh", isa_extension_t::EXT_ZKSH)
        .value("zkr", isa_extension_t::EXT_ZKR)
        .value("zmmul", isa_extension_t::EXT_ZMMUL)
        .value("zvfh", isa_extension_t::EXT_ZVFH)
        .value("zvfhmin", isa_extension_t::EXT_ZVFHMIN)
        .value("smepmp", isa_extension_t::EXT_SMEPMP)
        .value("smstateen", isa_extension_t::EXT_SMSTATEEN)
        .value("smrnmi", isa_extension_t::EXT_SMRNMI)
        .value("sscofpmf", isa_extension_t::EXT_SSCOFPMF)
        .value("svadu", isa_extension_t::EXT_SVADU)
        .value("svnapot", isa_extension_t::EXT_SVNAPOT)
        .value("svpbmt", isa_extension_t::EXT_SVPBMT)
        .value("svinval", isa_extension_t::EXT_SVINVAL)
        .value("zdinx", isa_extension_t::EXT_ZDINX)
        .value("zfa", isa_extension_t::EXT_ZFA)
        .value("zfbfmin", isa_extension_t::EXT_ZFBFMIN)
        .value("zfinx", isa_extension_t::EXT_ZFINX)
        .value("zhinx", isa_extension_t::EXT_ZHINX)
        .value("zhinxmin", isa_extension_t::EXT_ZHINXMIN)
        .value("zicbom", isa_extension_t::EXT_ZICBOM)
        .value("zicboz", isa_extension_t::EXT_ZICBOZ)
        .value("zicntr", isa_extension_t::EXT_ZICNTR)
        .value("zicond", isa_extension_t::EXT_ZICOND)
        .value("zihpm", isa_extension_t::EXT_ZIHPM)
        .value("zilsd", isa_extension_t::EXT_ZILSD)
        .value("zvbb", isa_extension_t::EXT_ZVBB)
        .value("zvbc", isa_extension_t::EXT_ZVBC)
        .value("zvfbfmin", isa_extension_t::EXT_ZVFBFMIN)
        .value("zvfbfwma", isa_extension_t::EXT_ZVFBFWMA)
        .value("zvkg", isa_extension_t::EXT_ZVKG)
        .value("zvkned", isa_extension_t::EXT_ZVKNED)
        .value("zvknha", isa_extension_t::EXT_ZVKNHA)
        .value("zvknhb", isa_extension_t::EXT_ZVKNHB)
        .value("zvksed", isa_extension_t::EXT_ZVKSED)
        .value("zvksh", isa_extension_t::EXT_ZVKSH)
        .value("sstc", isa_extension_t::EXT_SSTC)
        .value("zaamo", isa_extension_t::EXT_ZAAMO)
        .value("zalrsc", isa_extension_t::EXT_ZALRSC)
        .value("zacas", isa_extension_t::EXT_ZACAS)
        .value("zabha", isa_extension_t::EXT_ZABHA)
        .value("zawrs", isa_extension_t::EXT_ZAWRS)
        .value("smcsrind", isa_extension_t::EXT_SMCSRIND)
        .value("sscsrind", isa_extension_t::EXT_SSCSRIND)
        .value("smcntrpmf", isa_extension_t::EXT_SMCNTRPMF)
        .value("zimop", isa_extension_t::EXT_ZIMOP)
        .value("zcmop", isa_extension_t::EXT_ZCMOP)
        .value("zalasr", isa_extension_t::EXT_ZALASR)
        .value("ssqosid", isa_extension_t::EXT_SSQOSID)
        .value("zicfilp", isa_extension_t::EXT_ZICFILP)
        .value("zicfiss", isa_extension_t::EXT_ZICFISS)
        .value("ssdbltrp", isa_extension_t::EXT_SSDBLTRP)
        .export_values();

    py::enum_<impl_extension_t>(mod_isa_parser, "impl_extension_t")
        .value("SV32", impl_extension_t::IMPL_MMU_SV32)
        .value("SV39", impl_extension_t::IMPL_MMU_SV39)
        .value("SV48", impl_extension_t::IMPL_MMU_SV48)
        .value("SV57", impl_extension_t::IMPL_MMU_SV57)
        .value("MMU_SBARE", impl_extension_t::IMPL_MMU_SBARE)
        .value("MMU", impl_extension_t::IMPL_MMU)
        .value("MMU_VMID", impl_extension_t::IMPL_MMU_VMID)
        .value("MMU_ASID", impl_extension_t::IMPL_MMU_ASID)
        .export_values();

    py::class_<isa_parser_t>(mod_isa_parser, "isa_parser_t")
        .def(py::init<const char *, const char *>(), py::arg("isa"),
             py::arg("priv"))
        .def_property_readonly("max_xlen", &isa_parser_t::get_max_xlen)
        .def_property_readonly("max_isa", &isa_parser_t::get_max_isa)
        .def_property_readonly("isa", &isa_parser_t::get_isa_string)
        .def("__contains__",
             py::overload_cast<unsigned char>(&isa_parser_t::extension_enabled,
                                              py::const_),
             py::arg("ext"))
        .def("__contains__",
             py::overload_cast<isa_extension_t>(
                 &isa_parser_t::extension_enabled, py::const_),
             py::arg("ext"))
        .def(
            "__contains__",
            [](const isa_parser_t &self, const std::string &ext) -> bool {
              const auto &exts = self.get_extensions();
              return exts.find(ext) != exts.end();
            },
            py::arg("ext"))
        .def_property_readonly("extensions", &isa_parser_t::get_extensions);
  }

  // riscv.mmu
  {
    auto mod_mmu = m.def_submodule("mmu");

    py::class_<mmu_t>(mod_mmu, "mmu_t")
        // load_reserved
        .def("load_reserved", &mmu_t::load_reserved<int32_t>, py::arg("addr"))
        .def("load_reserved", &mmu_t::load_reserved<int64_t>, py::arg("addr"))
        .def("load_reserved_32", &mmu_t::load_reserved<int32_t>, py::arg("addr"))
        .def("load_reserved_64", &mmu_t::load_reserved<int64_t>, py::arg("addr"))
        // store_conditional
        .def("store_conditional", &mmu_t::store_conditional<uint32_t>, py::arg("addr"), py::arg("val"))
        .def("store_conditional", &mmu_t::store_conditional<uint64_t>, py::arg("addr"), py::arg("val"))
        .def("store_conditional_32", &mmu_t::store_conditional<uint32_t>, py::arg("addr"), py::arg("val"))
        .def("store_conditional_64", &mmu_t::store_conditional<uint64_t>, py::arg("addr"), py::arg("val"));
  }

  // riscv.processor
  {
    auto mod_processor = m.def_submodule("processor");

    using xpr_regfile_t = regfile_t<reg_t, NXPR, true>;
    py::class_<xpr_regfile_t>(mod_processor, "xpr_regfile_t")
        .def(py::init())
        .def("write", &xpr_regfile_t::write, py::arg("i"), py::arg("value"))
        .def("__getitem__", &xpr_regfile_t::operator[], py::arg("i"))
        .def("reset", &xpr_regfile_t::reset);

    using fpr_regfile_t = regfile_t<freg_t, NFPR, false>;
    py::class_<fpr_regfile_t>(mod_processor, "fpr_regfile_t")
        .def(py::init())
        .def("write", &fpr_regfile_t::write, py::arg("i"), py::arg("value"))
        .def("__getitem__", &fpr_regfile_t::operator[], py::arg("i"))
        .def("reset", &fpr_regfile_t::reset);

    py::class_<py_commit_log_reg_t>(mod_processor, "commit_log_reg_t")
        .def("__len__", &py_commit_log_reg_t::len)
        .def("__contains__", &py_commit_log_reg_t::contains)
        .def("__getitem__", &py_commit_log_reg_t::getitem,
             py::return_value_policy::reference)
        .def("__setitem__", py::overload_cast<reg_t, freg_t>(&py_commit_log_reg_t::setitem))
        .def("__setitem__", py::overload_cast<reg_t, py::tuple>(&py_commit_log_reg_t::setitem))
        .def("__delitem__", &py_commit_log_reg_t::delitem)
        .def("__repr__", &py_commit_log_reg_t::repr);

    py::class_<py_commit_log_mem_t>(mod_processor, "commit_log_mem_t")
        .def("__len__", &py_commit_log_mem_t::len)
        .def("__getitem__", &py_commit_log_mem_t::getitem,
             py::return_value_policy::reference)
        .def("append", &py_commit_log_mem_t::append)
        .def("__repr__", &py_commit_log_mem_t::repr);

    py::class_<state_t>(mod_processor, "state_t")
        .def(py::init())
        // state_t members
        .def_readwrite("pc", &state_t::pc)
        .def_readonly("XPR", &state_t::XPR,
                      py::return_value_policy::reference_internal)
        .def_readonly("FPR", &state_t::FPR,
                      py::return_value_policy::reference_internal)
        .def_readonly("prv", &state_t::prv)
        .def_readonly("prev_prv", &state_t::prev_prv)
        .def_readonly("prv_changed", &state_t::prv_changed)
        .def_readonly("v", &state_t::v)
        .def_readonly("prev_v", &state_t::prev_v)
        .def_readonly("v_changed", &state_t::v_changed)
        .def_readonly("last_inst_priv", &state_t::last_inst_priv)
        .def_readonly("last_inst_xlen", &state_t::last_inst_xlen)
        .def_readonly("last_inst_flen", &state_t::last_inst_flen)
        // commit logs
        .def_property_readonly("log_reg_write", [](state_t& self) {
            return new py_commit_log_reg_t(self.log_reg_write);
        }, py::return_value_policy::reference_internal)
        .def_property_readonly("log_mem_read",[](state_t& self) {
            return new py_commit_log_mem_t(self.log_mem_read);
        }, py::return_value_policy::reference_internal)
        .def_property_readonly("log_mem_write", [](state_t& self) {
            return new py_commit_log_mem_t(self.log_mem_write);
        }, py::return_value_policy::reference_internal)
        // state_t methods
        .def(
            "add_csr",
            [](state_t &self, reg_t addr, py::object py_csr) {
              auto raw_csr =
                  PythonBridge::getInstance().track<py_csr_t *>(py_csr);
              self.add_csr(addr, raw_csr->shared_from_this());
            },
            py::arg("addr"), py::arg("csr"))
        .def("reset", &state_t::reset, py::arg("proc"), py::arg("max_isa"));

    py::class_<processor_t>(mod_processor, "processor_t")
        .def_property_readonly("id", &processor_t::get_id)
        // csr
        .def("get_csr",
             py::overload_cast<int, insn_t, bool, bool>(&processor_t::get_csr),
             py::arg("which"), py::arg("insn"), py::arg("write"),
             py::arg("peek"))
        .def("get_csr", py::overload_cast<int>(&processor_t::get_csr),
             py::arg("which"))
        .def("put_csr", &processor_t::put_csr, py::arg("which"), py::arg("val"))
        // mmu
        .def_property_readonly("mmu", &processor_t::get_mmu, 
                               py::return_value_policy::reference_internal)
        // state
        .def_property_readonly("state", &processor_t::get_state,
                               py::return_value_policy::reference_internal)
        // instruction
        .def("register_base_insn", &processor_t::register_base_insn,
             py::arg("insn"))
        .def("register_custom_insn", &processor_t::register_custom_insn,
             py::arg("insn"))
        // extension
        .def("register_extension", &processor_t::register_extension,
             py::arg("x"))
        .def("get_extension", py::overload_cast<>(&processor_t::get_extension))
        .def("get_extension",
             py::overload_cast<const char *>(&processor_t::get_extension),
             py::arg("name"))
        // get address of processor_t *
        .def_static("addressof",
                    [](py::object proc) -> uint64_t {
                      return reinterpret_cast<uint64_t>(
                          py::cast<processor_t *>(proc));
                    })
        // instantiate py::object from processor_t *
        .def_static(
            "from_address",
            [](uint64_t addr) -> py::object {
              return py::cast(reinterpret_cast<processor_t *>(addr));
            },
            py::arg("proc"));

    // insn_func_t bindings

    using namespace py::literals;

    auto ctypes = py::module_::import("ctypes");
    auto typing = py::module_::import("typing");

    auto symb = py::dict(
        // from ctypes import CFUNCTYPE, c_void_p, c_uint64, py_object
        "CFUNCTYPE"_a = ctypes.attr("CFUNCTYPE"),
        "c_void_p"_a = ctypes.attr("c_void_p"),
        "c_uint64"_a = ctypes.attr("c_uint64"),
        "py_object"_a = ctypes.attr("py_object"),
        // from typing import Callable
        "Callable"_a = typing.attr("Callable"),
        // from ..decode import insn_t
        "insn_t"_a = mod_decode.attr("insn_t"),
        // from . import processor_t
        "processor_t"_a = mod_processor.attr("processor_t"));

    py::exec(R"(
      insn_func_ctype = CFUNCTYPE(c_uint64, c_void_p, c_uint64, c_uint64)

      def insn_func_py2ct(f: Callable[[processor_t, insn_t, int], int]) -> insn_func_ctype:
          @insn_func_ctype
          def py2ct(p: int, insn: c_uint64, pc: int) -> int:
              return f(processor_t.from_address(p), insn_t(insn), pc)
          return py2ct

      def insn_func_ct2py(f: insn_func_ctype) -> Callable[[processor_t, insn_t, int], int]:
          def ct2py(p: processor_t, insn: insn_t, pc: int) -> int:
              return f(processor_t.addressof(p), int.from_bytes(insn.bits, "little"), pc)
          return ct2py
    )",
             symb);

    mod_processor.attr("insn_func_ctype") = symb["insn_func_ctype"];
    mod_processor.attr("insn_func_ctype").doc() =
        "Prototype of C++ function pointer to insn_func_t";

    mod_processor.attr("insn_func_py2ct") = symb["insn_func_py2ct"];
    mod_processor.attr("insn_func_py2ct").doc() =
        "Decorator for Python -> C++ adaption of insn_func_t";

    mod_processor.attr("insn_func_ct2py") = symb["insn_func_ct2py"];
    mod_processor.attr("insn_func_ct2py").doc() =
        "Decorator for C++ -> Python adaption of insn_func_t";

    mod_processor.attr("illegal_instruction") = py::cpp_function(
        [](processor_t *p, insn_t insn, reg_t pc) -> reg_t {
          return ::illegal_instruction(p, insn, pc);
        },
        py::arg("p"), py::arg("insn"), py::arg("pc"));

    py::class_<insn_desc_t>(mod_processor, "insn_desc_t")
        .def(py::init(&py_insn_desc_t_create), py::keep_alive<1, 2>(),
             py::arg("match"), py::arg("mask"), py::arg("fast_rv32i"),
             py::arg("fast_rv64i"), py::arg("fast_rv32e"),
             py::arg("fast_rv64e"), py::arg("logged_rv32i"),
             py::arg("logged_rv64i"), py::arg("logged_rv32e"),
             py::arg("logged_rv64e"))
        // members
        .def_readonly("match", &insn_desc_t::match)
        .def_readonly("mask", &insn_desc_t::mask)
        // function pointer members
        .def_property_readonly(
            "fast_rv32i",
            [](const insn_desc_t &self) -> py::function {
              auto mod = py::module_::import("riscv._riscv.processor");
              auto ct2py = mod.attr("insn_func_ct2py");
              auto ctypeof = mod.attr("insn_func_ctype");
              return ct2py(
                  ctypeof(reinterpret_cast<uint64_t>(self.fast_rv32i)));
            })
        .def_property_readonly(
            "fast_rv64i",
            [](const insn_desc_t &self) -> py::function {
              auto mod = py::module_::import("riscv._riscv.processor");
              auto ct2py = mod.attr("insn_func_ct2py");
              auto ctypeof = mod.attr("insn_func_ctype");
              return ct2py(
                  ctypeof(reinterpret_cast<uint64_t>(self.fast_rv64i)));
            })
        .def_property_readonly(
            "fast_rv32e",
            [](const insn_desc_t &self) -> py::function {
              auto mod = py::module_::import("riscv._riscv.processor");
              auto ct2py = mod.attr("insn_func_ct2py");
              auto ctypeof = mod.attr("insn_func_ctype");
              return ct2py(
                  ctypeof(reinterpret_cast<uint64_t>(self.fast_rv32e)));
            })
        .def_property_readonly(
            "fast_rv64e",
            [](const insn_desc_t &self) -> py::function {
              auto mod = py::module_::import("riscv._riscv.processor");
              auto ct2py = mod.attr("insn_func_ct2py");
              auto ctypeof = mod.attr("insn_func_ctype");
              return ct2py(
                  ctypeof(reinterpret_cast<uint64_t>(self.fast_rv64e)));
            })
        .def_property_readonly(
            "logged_rv32i",
            [](const insn_desc_t &self) -> py::function {
              auto mod = py::module_::import("riscv._riscv.processor");
              auto ct2py = mod.attr("insn_func_ct2py");
              auto ctypeof = mod.attr("insn_func_ctype");
              return ct2py(
                  ctypeof(reinterpret_cast<uint64_t>(self.logged_rv32i)));
            })
        .def_property_readonly(
            "logged_rv64i",
            [](const insn_desc_t &self) -> py::function {
              auto mod = py::module_::import("riscv._riscv.processor");
              auto ct2py = mod.attr("insn_func_ct2py");
              auto ctypeof = mod.attr("insn_func_ctype");
              return ct2py(
                  ctypeof(reinterpret_cast<uint64_t>(self.logged_rv64i)));
            })
        .def_property_readonly(
            "logged_rv32e",
            [](const insn_desc_t &self) -> py::function {
              auto mod = py::module_::import("riscv._riscv.processor");
              auto ct2py = mod.attr("insn_func_ct2py");
              auto ctypeof = mod.attr("insn_func_ctype");
              return ct2py(
                  ctypeof(reinterpret_cast<uint64_t>(self.logged_rv32e)));
            })
        .def_property_readonly(
            "logged_rv64e",
            [](const insn_desc_t &self) -> py::function {
              auto mod = py::module_::import("riscv._riscv.processor");
              auto ct2py = mod.attr("insn_func_ct2py");
              auto ctypeof = mod.attr("insn_func_ctype");
              return ct2py(
                  ctypeof(reinterpret_cast<uint64_t>(self.logged_rv64e)));
            })
        .def(
            "func",
            [](const insn_desc_t &self, int xlen, bool rve,
               bool logged) -> py::function {
              auto mod = py::module_::import("riscv._riscv.processor");
              auto ct2py = mod.attr("insn_func_ct2py");
              auto ctypeof = mod.attr("insn_func_ctype");
              return ct2py(ctypeof(
                  reinterpret_cast<uint64_t>(self.func(xlen, rve, logged))));
            },
            py::arg("xlen"), py::arg("rve"), py::arg("logged"))
        // static members
        .def_readonly_static("illegal_instruction",
                             &insn_desc_t::illegal_instruction);
  }

  // riscv.htif
  {
    auto mod_htif = m.def_submodule("htif");

    py::class_<htif_t>(mod_htif, "htif_t")
        .def_property_readonly("tohost_addr", &htif_t::get_tohost_addr)
        .def_property_readonly("fromhost_addr", &htif_t::get_fromhost_addr);
  }

  // riscv.simif
  {
    auto mod_simif = m.def_submodule("simif");

    py::class_<simif_t>(mod_simif, "simif_t");
  }

  // riscv.sim
  {
    auto mod_sim = m.def_submodule("sim");

    py::class_<sim_t, py_sim_t, htif_t, simif_t>(mod_sim, "sim_t",
                                                 py::multiple_inheritance())
        .def(py::init(&py_sim_t::create), py::kw_only(), py::arg("cfg"),
             py::arg("halted"), py::arg("plugin_device_factories"),
             py::arg("args"), py::arg("dm_config"))
        .def_property_readonly("cfg", &sim_t::get_cfg)
        .def_property_readonly("plic", &sim_t::get_intctrl)
        .def_property_readonly("nprocs", &sim_t::nprocs)
        .def("get_core", py::overload_cast<size_t>(&sim_t::get_core),
             py::arg("i"), py::return_value_policy::reference_internal)
        .def("get_dts", &sim_t::get_dts)
        .def("set_debug", &sim_t::set_debug, py::arg("value"))
        .def("configure_log", &sim_t::configure_log, py::arg("enable_log"),
             py::arg("enable_commitlog"))
        .def("run", &sim_t::run)
        .def("stop", &sim_t::stop)
        .def("proc_reset", &sim_t::proc_reset, py::arg("id"));
  }

  // riscv.test
  {
    auto mod_test = m.def_submodule("test");

    mod_test.def("_test_mmio_load", &py_mmio_load, py::arg("device"),
                 py::arg("addr"), py::arg("len"));
    mod_test.def("_test_mmio_store", &py_mmio_store, py::arg("device"),
                 py::arg("addr"), py::arg("data"));
    mod_test.def("_test_mmio_tick", &py_mmio_tick, py::arg("device"),
                 py::arg("rtc_ticks"));

    mod_test.def("_test_mmio_parse_from_fdt", &py_mmio_parse_from_fdt,
                 py::arg("factory"), py::arg("fdt"), py::arg("sim"),
                 py::return_value_policy::reference);
    mod_test.def("_test_mmio_generate_dts", &py_mmio_generate_dts,
                 py::arg("factory"), py::arg("sim"));
  }

  // fesvr.term
  {
    auto mod_fesvr = m.def_submodule("fesvr", "Frontend Server");

    auto mod_term = mod_fesvr.def_submodule("term", "Canonical Terminal");

    mod_term.def("read", &py_canonical_terminal_t::read);
    mod_term.def("write", &py_canonical_terminal_t::write);
  }
}

PythonBridge PythonBridge::singleton;
