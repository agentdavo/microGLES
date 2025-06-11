import sys

inp = sys.argv[1]
out = sys.argv[2]

with open(inp) as f:
    funcs = [line.strip() for line in f if line.strip()]

with open(out, 'w') as f:
    f.write('#include "tests.h"\n')
    for func in funcs:
        test_name = f'test_call_{func}'
        f.write(f'int {test_name}(void) {{ {func}(0); CHECK_GLError(GL_NO_ERROR); return 1; }}\n')
    f.write('\nstatic const struct Test tests[] = {\n')
    for func in funcs:
        f.write(f'    {{"call_{func}", test_call_{func}}},\n')
    f.write('};\n')
    f.write('const struct Test *get_autogen_tests(size_t *count)\n{\n')
    f.write('    *count = sizeof(tests)/sizeof(tests[0]);\n    return tests;\n}\n')
