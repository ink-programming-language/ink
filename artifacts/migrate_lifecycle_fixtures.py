"""Make the former aggregate-construction test fixtures declare their constructors."""

import ast
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[1]
TOKEN = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|[A-Za-z_]\w*|\S', re.S)


def migrate(source, multiline=False):
    tokens = [match for match in TOKEN.finditer(source) if not match.group().startswith(('//', '/*'))]
    edits = []
    for pos, token in enumerate(tokens[:-2]):
        if token.group() != 'class' or tokens[pos + 2].group() != '{':
            continue
        start = pos + 2
        depth = 1
        end = start + 1
        while end < len(tokens) and depth:
            depth += (tokens[end].group() == '{') - (tokens[end].group() == '}')
            end += 1
        if depth:
            continue
        end -= 1
        fields = []
        has_init = False
        depth = 0
        index = start + 1
        while index < end:
            word = tokens[index].group()
            if depth == 0 and word == 'func' and tokens[index + 1].group() == '__init__':
                has_init = True
            if depth == 0 and word == 'field' and tokens[index + 2].group() == ':':
                name = tokens[index + 1].group()
                begin_type = index + 3
                cursor = begin_type
                nesting = 0
                while cursor < end:
                    item = tokens[cursor].group()
                    if nesting == 0 and item in ('=', ';'):
                        break
                    nesting += item in ('[', '(')
                    nesting -= item in (']', ')')
                    cursor += 1
                type_text = source[tokens[begin_type].start():tokens[cursor].start()].strip()
                fields.append((name, type_text, tokens[cursor].group() == '=', index > 0 and tokens[index - 1].group() == 'private'))
            depth += (word == '{') - (word == '}')
            index += 1
        if has_init or not fields:
            continue
        minimum = max((index + 1 for index, field in enumerate(fields) if not field[2]), default=0)
        methods = []
        for count in range(minimum, len(fields) + 1):
            parameters = ', '.join(f'Initial{name}: {type_text}' for name, type_text, _, _ in fields[:count])
            assignments = [f'this.{name} = Initial{name};' for name, _, _, _ in fields[:count]]
            visibility = 'private ' if any(field[3] for field in fields[:count]) else ''
            if multiline:
                line_begin = source.rfind('\n', 0, token.start()) + 1
                indent = re.match(r'\s*', source[line_begin:token.start()]).group()
                member = indent + '  '
                method = '\n' + member + f'{visibility}func __init__({parameters}): void\n' + member + '{\n'
                method += ''.join(member + '  ' + assignment + '\n' for assignment in assignments)
                method += member + '}\n'
            else:
                method = f' {visibility}func __init__({parameters}): void {{ ' + ' '.join(assignments) + ' } '
            methods.append(method)
        edits.append((tokens[end].start(), ''.join(methods)))
    for offset, text in sorted(edits, reverse=True):
        source = source[:offset] + text + source[offset:]
    return source


paths = list((ROOT / 'src/testcase/backend/programs').rglob('*.ink'))
paths += list((ROOT / 'src/testcase/execution/class_programs').rglob('*.ink'))
paths += list((ROOT / 'src/testcase/execution/multifile').rglob('*.ink'))
for path in paths:
    if path.name in ('missing_initializer.ink', 'const_method.ink'):
        continue
    original = path.read_text(encoding='utf-8')
    updated = migrate(original, True)
    if updated != original:
        path.write_text(updated, encoding='utf-8', newline='\n')
        print(path.relative_to(ROOT))

for path in (ROOT / 'src/testcase').rglob('*_test.cpp'):
    if '/semantic/' not in path.as_posix() and path.name != 'symbol_mangling_test.cpp':
        continue
    original = path.read_text(encoding='utf-8')
    updated_lines = []
    lifecycle_tests = False
    for line in original.splitlines(keepends=True):
        lifecycle_tests |= '// Only declared constructor signatures' in line
        if lifecycle_tests or 'expectClassDiagnostic(' in line:
            updated_lines.append(line)
            continue

        def update_literal(match):
            literal = match.group()
            if 'field ' not in literal or 'class ' not in literal:
                return literal
            try:
                source = ast.literal_eval(literal)
            except (ValueError, SyntaxError):
                return literal
            updated = migrate(source)
            if updated == source:
                return literal
            return '"' + updated.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n').replace('\r', '\\r').replace('\t', '\\t') + '"'

        updated_lines.append(re.sub(r'"(?:\\.|[^"\\])*"', update_literal, line))
    updated = ''.join(updated_lines)
    if updated != original:
        path.write_text(updated, encoding='utf-8', newline='\n')
        print(path.relative_to(ROOT))
