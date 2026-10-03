const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { before, test } = require('node:test');
const textmate = require('vscode-textmate');
const oniguruma = require('vscode-oniguruma');

const repositoryRoot = path.resolve(__dirname, '../../..');
const grammarPath = path.resolve(__dirname, '../Ink.tmbundle/Syntaxes/Ink.tmLanguage');
let grammar;

before(async () => {
  const wasm = fs.readFileSync(require.resolve('vscode-oniguruma/release/onig.wasm'));
  await oniguruma.loadWASM(wasm.buffer.slice(wasm.byteOffset, wasm.byteOffset + wasm.byteLength));
  const registry = new textmate.Registry({ onigLib: Promise.resolve({ createOnigScanner: patterns => new oniguruma.OnigScanner(patterns), createOnigString: text => new oniguruma.OnigString(text) }), loadGrammar: async () => textmate.parseRawGrammar(fs.readFileSync(grammarPath, 'utf8'), grammarPath) });
  grammar = await registry.loadGrammar('source.inklang');
  assert.ok(grammar);
});

function tokenize(source) {
  let state = textmate.INITIAL;
  return source.split(/\r\n|\r|\n/).map(line => {
    const result = grammar.tokenizeLine(line, state);
    assert.equal(result.stoppedEarly, false);
    state = result.ruleStack;
    return { line, tokens: result.tokens, state };
  });
}

function scopesAt(row, text) {
  const offset = row.line.indexOf(text);
  assert.ok(offset >= 0, `Missing ${JSON.stringify(text)} in ${JSON.stringify(row.line)}`);
  return row.tokens.find(token => token.startIndex <= offset && token.endIndex > offset).scopes;
}

function expectScope(row, text, prefix) {
  assert.ok(scopesAt(row, text).some(scope => scope.startsWith(prefix)), `${JSON.stringify(text)}: expected ${prefix}, got ${scopesAt(row, text).join(', ')}`);
}

// Keywords stay synchronized with the compiler and never split Unicode identifiers.
test('all compiler keywords and Unicode identifier boundaries', () => {
  const definitions = fs.readFileSync(path.join(repositoryRoot, 'src/include/ink/tokenizer/token.def'), 'utf8');
  const keywords = [...definitions.matchAll(/^INK_KEYWORD\([^,]+, "[^"]+", "([^"]+)"\)/gm)].map(match => match[1]);
  for (const keyword of keywords) {
    assert.ok(scopesAt(tokenize(keyword)[0], keyword).some(scope => /^(keyword\.control|storage\.)/.test(scope)), keyword);
    for (const name of [keyword + 'Name', keyword + '变量', '变量' + keyword, 'e\u0301' + keyword, keyword + '\u0301']) {
      assert.ok(tokenize(name)[0].tokens.every(token => !token.scopes.some(scope => /^(keyword\.|storage\.)/.test(scope))), name);
    }
  }
});

// Visibility modifiers receive their own scope without splitting longer or underscore-delimited identifiers.
test('visibility modifiers and identifier boundaries', () => {
  for (const modifier of ['public', 'private']) {
    const row = tokenize(`${modifier} func visible(): i32`)[0];
    expectScope(row, modifier, 'storage.modifier');
    expectScope(row, 'visible', 'entity.name.function');
    for (const name of [`_${modifier}`, `${modifier}_name`, `${modifier}1`, `${modifier}Name`]) {
      expectScope(tokenize(name)[0], name, 'variable.other');
      assert.ok(tokenize(name)[0].tokens.every(token => !token.scopes.some(scope => scope.startsWith('storage.modifier'))), name);
    }
  }
});

// Native binding keywords coexist with module imports and the retired extern spelling is an ordinary identifier.
test('native import and export declarations', () => {
  const imported = tokenize('private import "C" func abs(Value: i32): i32;')[0];
  expectScope(imported, 'import', 'keyword.control');
  expectScope(imported, 'abs', 'entity.name.function');
  const exported = tokenize('public export "C" func sum(A: i32, B: i32): i32')[0];
  expectScope(exported, 'export', 'storage.modifier');
  expectScope(exported, 'sum', 'entity.name.function');
  expectScope(tokenize('extern')[0], 'extern', 'variable.other');
});

// Only currently supported builtin spellings receive builtin type or constant scopes.
test('builtin types and constants', () => {
  for (const name of ['void', 'bool', 'type', 'i8', 'u16', 'i32', 'u64', 'i128', 'u128', 'f16', 'f32', 'f64']) expectScope(tokenize(name)[0], name, 'support.type.builtin');
  for (const name of ['true', 'false']) expectScope(tokenize(name)[0], name, 'constant.language');
  for (const name of ['i24', 'i032', 'f128', 'usize', 'null', 'falsehood', 'bool变量']) expectScope(tokenize(name)[0], name, 'variable.other');
});

// Names after declarations and at call sites are styled without semantic analysis.
test('declaration and call names', () => {
  expectScope(tokenize('func 求和(A: i32): i32')[0], '求和', 'entity.name.function');
  expectScope(tokenize('class Example;')[0], 'Example', 'entity.name.type');
  expectScope(tokenize('求和(1);')[0], '求和', 'entity.name.function');
});

// Raw prefixes are recognized only at token boundaries and do not escape closing quotes.
test('raw literal prefixes and raw closing delimiters', () => {
  for (const source of ['r"\\n"', 'R"\\n"', "r'\\'", "R'\\'"]) {
    const row = tokenize(source)[0];
    expectScope(row, source, 'string.quoted');
    assert.ok(row.tokens.every(token => !token.scopes.some(scope => scope.startsWith('constant.character.escape'))));
  }
  for (const prefix of ['xr', '变量r', 'r ']) {
    const row = tokenize(prefix + '"\\n"')[0];
    expectScope(row, '\\n', 'constant.character.escape');
    assert.ok(row.tokens.every(token => !token.scopes.some(scope => scope.includes('.raw.'))));
  }
  expectScope(tokenize('r"a\\" var After;')[0], 'var', 'storage.type');
});

// Escapes protect quotes; a pair of backslashes does not protect the next delimiter.
test('ordinary escapes and quote parity', () => {
  const escapedQuote = tokenize('"a\\"b" var After;')[0];
  expectScope(escapedQuote, 'b', 'string.quoted.double');
  expectScope(escapedQuote, 'var', 'storage.type');
  expectScope(tokenize('"a\\\\" var After;')[0], 'var', 'storage.type');
  for (const escape of ['\\n', '\\xFF', '\\u4E2D', '\\U0001F600', '\\N{LATIN CAPITAL LETTER A}']) expectScope(tokenize('"' + escape + '"')[0], escape, 'constant.character.escape');
  expectScope(tokenize('"\\q"')[0], '\\q', 'invalid.illegal.escape');
  expectScope(tokenize('"\\N{"} var After;')[0], 'var', 'storage.type');
});

// Short literals recover at a newline, except ordinary escaped physical newlines.
test('short literal recovery and line continuation', () => {
  for (const newline of ['\n', '\r\n', '\r']) {
    for (const quote of ['"', "'"]) {
      expectScope(tokenize(quote + 'a\\' + newline + 'b' + quote)[1], 'b', 'string.quoted');
      expectScope(tokenize(quote + 'a' + newline + 'var After;')[1], 'var', 'storage.type');
      expectScope(tokenize('r' + quote + 'a\\' + newline + 'var After;')[1], 'var', 'storage.type');
      expectScope(tokenize(quote + 'a\\\\' + newline + 'var After;')[1], 'var', 'storage.type');
    }
  }
});

// Triple double quotes span lines while triple single quotes do not define a multiline literal.
test('triple quoted strings', () => {
  for (const prefix of ['', 'r', 'R']) {
    const rows = tokenize(prefix + '"""start\nif // still string\nend""" var After;');
    expectScope(rows[1], 'if', 'string.quoted.triple.double');
    expectScope(rows[2], 'var', 'storage.type');
  }
  expectScope(tokenize("'''start\nvar After;")[1], 'var', 'storage.type');
});

// Comments do not nest or continue via backslash, and quoted comment markers are inert.
test('comment boundaries and strings', () => {
  const rows = tokenize('// comment\\\nif (true) {}\n/* outer /* inner */ var After;');
  expectScope(rows[0], 'comment', 'comment.line');
  expectScope(rows[1], 'if', 'keyword.control');
  expectScope(rows[2], 'var', 'storage.type');
  expectScope(tokenize('"// /* */"')[0], '//', 'string.quoted.double');
  expectScope(tokenize('/* "quoted" */')[0], 'quoted', 'comment.block');
});

// Malformed numbers must not receive a valid numeric prefix scope.
test('numeric spelling boundaries', () => {
  for (const number of ['0', '01', '42', '0b101', '0O77', '0xFF', '1e3', '1.0', '1.0e-3']) expectScope(tokenize(number)[0], number, 'constant.numeric');
  for (const number of ['12abc', '12变量', '0x', '0xG', '0b102', '1e+', '1_000', '1.2foo', '0x1p2', '0x1.2', '0x1.foo', '0x1.2p-3']) assert.ok(tokenize(number)[0].tokens.every(token => !token.scopes.some(scope => scope.startsWith('constant.numeric'))), number);
  expectScope(tokenize('.5')[0], '.', 'punctuation.separator');
  expectScope(tokenize('.5')[0], '5', 'constant.numeric');
  expectScope(tokenize('1.')[0], '1', 'constant.numeric');
  expectScope(tokenize('1.')[0], '.', 'punctuation.separator');
  expectScope(tokenize('-1e-3')[0], '-', 'keyword.operator');
});

// Valid project examples must finish with no leaked multiline string or comment state.
test('execution source corpus has no leaked lexical state', () => {
  const directory = path.join(repositoryRoot, 'src/testcase/execution/programs');
  const files = fs.readdirSync(directory).filter(name => name.endsWith('.ink'));
  assert.ok(files.length > 0);
  for (const file of files) {
    const rows = tokenize(fs.readFileSync(path.join(directory, file), 'utf8'));
    assert.equal(rows.at(-1).state.depth, 1, file);
  }
});
