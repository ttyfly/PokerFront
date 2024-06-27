from code_gen_base import CodeGen, CodeGenContainer


class CppCodeGen(CodeGenContainer):
    def __init__(self, name, namespace=None):
        super().__init__(name, 0)
        self.namespace = namespace

    def iter_lines(self):
        indent_count = 0
        if self.namespace is not None:
            yield 'namespace %s' % self.namespace
            yield '{'
            indent_count += 1
        for content in self.iter_contents():
            for line in content.iter_lines():
                yield '\t' * indent_count + line
        if self.namespace is not None:
            yield '}'
            indent_count -= 1

    def add_constexpr(self, name, typename, value):
        self.add(CppConstexpr(name, typename, value))


class CppVariable(CodeGen):
    def __init__(self, name, typename, value):
        super().__init__(name, 2)
        self.name = name
        self.typename = typename
        self.value = value

    def iter_lines(self):
        yield '%s %s = %s;' % (self.typename, self.name, self.value)


class CppConstexpr(CodeGen):
    def __init__(self, name, typename, value):
        super().__init__(name, 1)
        self.typename = typename
        self.value = value

    def iter_lines(self):
        yield 'constexpr %s %s = %s;' % (self.typename, self.name, self.value)


class CppFunction(CodeGen):
    def __init__(self, name: str, signature: str, plain_text: str):
        super().__init__(name, 50)
        self.signature = signature
        self.plain_text = plain_text

    def iter_lines(self):
        yield self.signature
        yield '{'
        for line in self.plain_text.split('\n'):
            yield '\t' + line.strip()
        yield '}'


class CppStruct(CodeGenContainer):
    def __init__(self, name):
        super().__init__(name, 100)

    def iter_lines(self):
        yield 'struct %s' % self.name
        yield '{'
        for content in self.iter_contents():
            for line in content.iter_lines():
                yield '\t' + line
        yield '}'
