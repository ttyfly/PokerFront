from __future__ import annotations
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from code_gen_base import CodeGen, CodeGenContainer
    from cpp_code_gen import CppCodeGen, CppStruct, CppFunction, CppVariable, CppConstexpr
