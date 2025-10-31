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
import abc
from typing import Type
from riscv.extension import extension_t, rocc_t, register_extension


__all__ = ['ISA', 'ROCC', 'register']


class ISA(extension_t):
    """
    ISA Abstract Base
    """

    @property
    @abc.abstractmethod
    def name(self) -> str:
        raise NotImplementedError("abstract class")

    def _name(self) -> str:
        # C++ -> Python call to extension_t::name() hits this
        return self.name


class ROCC(rocc_t, ISA):
    """
    RoCC Abstract Base
    """


def register(ext_name: str):
    """
    Decorator for registering ISA/RoCC extension
    """

    # if find_extension(name) is not None:
    #     raise KeyError(f"Extension '{name}' already registered")

    def isa_decorator(ext_cls: Type[extension_t]):

        class MyISA(ext_cls):

            @property
            def name(self) -> str:
                return ext_name

        register_extension(ext_name, MyISA)
        return MyISA

    return isa_decorator
