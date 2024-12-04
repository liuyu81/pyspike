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
import argparse
import os
import pathlib
import shutil
import sys
import sysconfig
from typing import Mapping, Sequence

from . import ENV_PYSPIKE_LIBS
from ._utils import find_python_library, find_bridge_library, find_spike_executable


__all__ = ["main", "entry"]


def main(args: Sequence[str], env: Mapping[str, str] = os.environ):
    # parse known and unknown spike cli arguments
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--extlib", action='append', dest='extlib', nargs=argparse.ZERO_OR_MORE)
    known_args, unknown_args = parser.parse_known_intermixed_args(args)

    # patch spike cli arguments to load pyspike libraries
    dll_ext = sysconfig.get_config_var("SHLIB_SUFFIX")
    clibs = [find_python_library(), find_bridge_library()]
    pylibs = [pathlib.Path(__file__).parent]
    for lib in known_args.extlib or []:
        for name in lib:
            fpath = pathlib.Path(name)
            # check if name is a C/C++ shared library
            if fpath.is_file() and fpath.suffix == dll_ext and fpath not in clibs:
                clibs.append(fpath.absolute())
                continue
            # assume everything else to be Python (ip) library
            pylibs.append(fpath)

    # execute vainilla spike with patched cli arguments
    spike_exec = find_spike_executable()
    os.execve(path=spike_exec, argv=[
        spike_exec,
        *[f"--extlib={ lib }" for lib in clibs],
        *unknown_args,
    ], env=dict(**env, **{
        ENV_PYSPIKE_LIBS: os.pathsep.join(map(str, pylibs)),
    }))


def entry():
    main(sys.argv[1:])


if __name__ == "__main__":
    entry()
