#
# Copyright 2024 WuXi EsionTech Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
import importlib
import sys
import types
import warnings

from ._utils import load_spike_library

try:
    from ._version import __version__
except ImportError:
    warnings.warn("Missing `riscv._version`, run `python -m setuptools_scm --force-write-version-files` to generate.")

__all__ = ["ENV_PYSPIKE_LIBS", "ENV_PYSPIKE_EXTS"]

ENV_PYSPIKE_LIBS = "PYSPIKE_LIBS"

ENV_PYSPIKE_EXTS = "PYSPIKE_EXTS"

# load spike runtime library (libriscv.so, libcustomext.so)
try:
    load_spike_library("riscv")
except RuntimeError:
    warnings.warn("Missing `libriscv.so`, run `python install -e '.[dev]' to build it.")
else:
    # import pyspike module (_riscv), and mount submodules _riscv.* to riscv.*
    try:
        _riscv: types.ModuleType = importlib.import_module('._riscv', __package__)
        self: types.ModuleType = sys.modules[__package__]
        for name in dir(_riscv):
            _attr = getattr(_riscv, name)
            if isinstance(_attr, types.ModuleType):
                sys.modules[f'{__package__}.{name}'] = _attr
    except ImportError as exc:
        warnings.warn("Missing `riscv._riscv`, run `python setup.py build_ext --inplace` to build it.")
