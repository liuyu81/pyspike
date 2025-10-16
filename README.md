# Python Bindings of Spike RISC-V ISA Simulator

```text
LIU Yu <liuy@huimtlab.org>
2025/10/16 (v0.0.5)
```

## Introduction

This project provides Python bindings for the [Spike RISC-V ISA Simulator](https://github.com/riscv-software-src/riscv-isa-sim). The Pythonic Spike (or PySpike) opens up Spike's C++ internals (such as RISC-V disassembler, processors, controllers, peripherals, etc.) for interoperation with Python scripts. It enables users to write ISA / RoCC extensions and MMIO device models in Python, and plug them into vanilla Spike for (co-)simulating complex hardware systems. Through integrating Spike more seamlessly into the Python ecosystem, PySpike aims to boost the agility of Python-based hardware verification tools and workflows.

PyPI package: [`spike`](https://pypi.org/project/spike/)


## Getting Started

PySpike requires: Python 3.8+.

Install the wheel package with `pip`.

```shell
$ pip install --pre spike
```

PySpike ships the original command-line tool `spike`, a.k.a *vanilla Spike*, within its wheel package. You can confirm its availability using,

```shell
$ spike --help
Spike RISC-V ISA Simulator 1.1.1-dev
...
```

There is also a 100%-compatible command-line wrapper called `pyspike`, with additional support for Python-based ISA / MMIO / RoCC extensions via `--extlib=<name>`.

```shell
$ pyspike \
    --isa=rv32imc_xmyisa --priv=m \
    --pc=0x90000000 \
    -m0x90000000:0x4000000 \
    --extlib=myisa.py \
    --extlib=mydev.py \
    --device=mydev,0x20000000 \
    tests/data/libc-printf_hello.elf
Hello, World!
```

### Quick ISA Extension

An ISA extension in PySpike is a Python class that inherits `riscv.isa.ISA`. It should implement a minimum of two methods: `get_instructions` and `get_disasms`. The former provides functional models of one or more custom instructions, and the latter provides their disassemblers. Other optional methods include `get_csrs` and `reset`, for providing custom *control state registers* （CSRs) and resetting extension states, respectively. Use decorator `@isa.register("myisa")` to register the extension under the name `myisa`.

```python
from typing import List
from riscv import isa
from riscv.disasm import disasm_insn_t
from riscv.processor import insn_desc_t

@isa.register("myisa")
class MyISA(isa.ISA):
    def __init__(self): ...
    def get_instructions(self, proc: processor_t) -> List[insn_desc_t]: ...
    def get_disasms(self, proc: processor_t) -> List[disasm_insn_t]: ...
    def get_csrs(self, proc: processor_t) -> List[csr_t]: ...
    def reset(self) -> None: ...
```

### Quick Device Model

Likewise to the ISA extension, an MMIO model in PySpike is a class that inherits `riscv.dev.MMIO`. It should implement a minimum of three methods: `__init__`, `load`, and `store`. The former initializes the model, the latter two handle memory read and write operations. Other optional methods include `size` and `tick`, for obtaining the size of memory-mapped address space, and shifting device states, respectively. Use decorator `@dev.register("mydev")` to register the model under the name `mydev`.

```python
from typing import Optional
from riscv import dev
from riscv.sim import sim_t

@dev.register("mydev")
class MyDEV(dev.MMIO):
    def __init__(self, sim: sim_t, args: Optional[str] = None): ...
    def load(self, addr: int, size: int) -> bytes: ...
    def store(self, addr: int, data: bytes) -> None: ...
    def size(self) -> int: ...
    def tick(self, rtc_ticks: int) -> None:
```

## Development

### Getting Source Code

```shell
$ git clone --recurse-submodules https://github.com/huimtlab/pyspike
$ cd pyspike
```

### Setting Up Develop Environment

```shell
$ python -m venv .venv
$ source .venv/bin/activate
(.venv) $ python -m pip install -e '.[dev]'
```

### Running Tests

```shell
(.venv) $ python -m pytest -v
```

### Packaging

```shell
(.venv) $ python -m build
```
