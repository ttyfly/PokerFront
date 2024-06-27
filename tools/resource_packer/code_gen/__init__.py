from code_gen.cpp_code_gen import CppCodeGen


def cpp(name, namespace=None):
    return CppCodeGen(name, namespace)
