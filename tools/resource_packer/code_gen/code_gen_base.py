from __future__ import annotations
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from typing import Iterable, Optional, Dict


class CodeGen:
    def __init__(self, name: str, priority: int):
        self.name = name
        self.priority = priority

    def iter_lines(self) -> Iterable[str]:
        raise StopIteration


class CodeGenContainer(CodeGen):
    def __init__(self, name: str, priority: int):
        super().__init__(name, priority)
        self.__contents: Optional[Dict[str, CodeGen]] = {}

    def add(self, content: CodeGen) -> None:
        self.__contents[content.name] = content

    def iter_contents(self) -> Iterable[CodeGen]:
        return sorted(self.__contents.values(), key=lambda x: (x.priority, x.name))
