from __future__ import annotations
import abc
import os.path
from typing import TYPE_CHECKING

import code_gen

if TYPE_CHECKING:
    from typing import List, Optional
    from code_gen.typing import CppCodeGen


class PackerBase(metaclass=abc.ABCMeta):
    def __init__(self, project_dir: str):
        self.project_dir = project_dir
        self.generated_code: Optional[CppCodeGen] = None

    def get_path(self, relative_path: str) -> str:
        return os.path.join(self.project_dir, relative_path)

    def generate_code(self, name) -> CppCodeGen:
        self.generated_code = code_gen.cpp(name, 'pf')
        return self.generated_code

    @abc.abstractmethod
    def pack(self) -> bytearray:
        pass

    @abc.abstractmethod
    def get_required_resources(self) -> List[str]:
        pass
