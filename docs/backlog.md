# Backlog of nice-to-have PySpike features and Spike improvements

```text
2025/11/04
```

## feat (pyspike): Pythonic DTS / DTB manip for `device_factory_t`

## feat (pyspike): Upstream compatibility test

**Problem**: PySpike integrates the upstream Spike as submodule (in `vendor/spike`). Once in a while, we want to pull the latest upstream Spike, and check its compatibility (with PySpike) by running PySpike's test suite.

**Call-for-contribution**: We need a compatibility test script that can temporarily update the Spike submodule, run PySpike's build & test workflow, and report compatibility results. The baseline is to run this compatibility against the tip of upstream's `master` branch. Ideally, we may also hope to iterate over pending pull requests of the upstream, run compatibility tests against each of them.

## fix (pyspike): Python object ref-count analysis

**Problem**: We use `PythonBridge::track()` to keep python objects  (such as callbacks) alive on the C++ side. 

## feat (pyspike): Built-in C++ extension build system

**Problem**: PySpike bundles Spike binaries, including C++ header files and the runtime library (`libriscv.so`). The library was compiled in [`manylinux2014`](https://github.com/pypa/manylinux) environment, and following the [name mangling](https://en.wikipedia.org/wiki/Name_mangling) dialects -- meaning you cannot really reuse a pre-compiled C++ Spike plugin with PySpike unless the plugin (shared library) was compiled in a `manylinux2014`-compatible environment.

**Call-for-contribution**: We are thinking of something like `pybind11`, to be implemented in PySPike's develop environment. One that can help users recompile their existing C++ plugins from source, ensuring ABI compatibility with the bundled Spike.

## feat (pyspike): Bundle `pk` and `bbl` with PySpike


## feat (pyspike): Compile test programs from source

**Problem**: PySpike uses pre-compiled RISC-V ELF's such as `huimt-lr_sc.elf` and `libc-printf_hello.elf` in tests. While we are willing to publicize the compiliation workflow, there were no binary distributions of RISC-V cross-compiling toolchain and we surely don't what to compile the entire compiler toolchain from source during CI. Fortunately, there is now [xPack Binary Development Tools
](https://github.com/xpack-dev-tools)

## fix (spike): Graceful `find_extension()` failures

**Problem**: Spike's builtin `find_extension()` function calls `exit()` if the extension is missing by the specified name.  This is not something we can override from the Python side.

## fix (spike): Programmatic single-stepping of `sim_t`

