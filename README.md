# Python Bindings of Spike RISC-V ISA Simulator

```text
LIU Yu <liuy@etech-inc.com>
2024/08/29 (v0.0.4)
```

## Introduction

This project provides Python bindings for the [Spike RISC-V ISA Simulator](https://github.com/riscv-software-src/riscv-isa-sim). The Pythonic Spike (or PySpike) opens up Spike's C++ internals (such as RISC-V disassembler, processors, controllers, peripherals, etc.) for interoperation with Python scripts. It enables users to write ISA / RoCC extensions and MMIO device models in Python, and plug them into vanilla Spike for (co-)simulating complex hardware systems. Through integrating Spike more seamlessly into the Python ecosystem, PySpike aims to boost the agility of Python-based hardware verification tools and workflows.

PyPI package: `spike`

## Get Started

PySpike requires: Python 3.8+ and vanilla Spike (commit [37b0dc0](https://github.com/riscv-software-src/riscv-isa-sim/commit/37b0dc0b52b5536ab19af3a7678f1a1cd8087942) or later).

### Install PySpike

1. Run the following command in your console,

```bash
$ pip install -U spike
```

2. Check that you installed the correct version

```bash
$ pyspike --help
Spike RISC-V ISA Simulator 1.1.1-dev
...
```

### Quick ISA Extension

An ISA extension in PySpike is a class that inherits from `riscv.insn.ISA`. It should implement a minimum of two methods: `get_instructions` and `get_disasms`. The former provides functional models of one or more RISC-V instructions, while the latter provides their disassemblers. A special decorator `@insn.register("myisa")` is used to register the extension under the name `myisa`.

```python
from typing import List
from riscv import insn
from riscv.disasm import disasm_insn_t
from riscv.processor import insn_desc_t

@insn.register("myisa")
class MyISA(insn.ISA):
    def __init__(self): ...
    def get_instructions(self) -> List[insn_desc_t]: ...
    def get_disasms(self) -> List[disasm_insn_t]: ...
    def reset(self) -> None: ...
```

### Quick MMIO Model

Likewise to the ISA extension, an MMIO model in PySpike is a class that inherits from `riscv.mmio.MMIO`. It should implement a minimum of three methods: `__init__`, `load`, and `store`. The former initializes the model, the latter two handle memory read and write operations. A special decorator `@mmio.register("mydev")` is used to register the model under the name `mydev`.

```python
from typing import Optional
from riscv import mmio
from riscv.sim import sim_t

@mmio.register("mydev")
class MyDEV(mmio.MMIO):
    def __init__(self, sim: sim_t, args: Optional[str] = None): ...
    def load(self, addr: int, size: int) -> bytes: ...
    def store(self, addr: int, data: bytes) -> None: ...
    def tick(self, rtc_ticks: int) -> None:
```

The PySpike provides a wrapper called `pyspike`. It is 100%-compatible with the command-line interface of vanilla Spike, with additional support for Python-based MMIO / RoCC extensions via `--extlib=<name>`.

```bash
$ pyspike \
    --isa=rv32imc --priv=m \
    --pc=0x90000000 \
    -m0x90000000:0x4000000 \
    --extlib=mydev.py \
    --device=mydev,0x20000000 \
    tests/data/libc-printf_hello.elf
Hello, World!
```

## Development

### Setup

```bash
$ python -m venv .venv
$ source .venv/bin/activate
(.venv) $ pip install -r requirements.txt
```

### Compile

```bash
(.venv) $ python setup.py build_ext --inplace
```

### Test

```bash
(.venv) $ pytest -v
```

### Package

```bash
(.venv) $ python -m build
```
