// Translated from solution.cpp.

func answer(v: dynamic) -> dynamic
{
  var s: dynamic = ["NO", "YES"];
  write(s[v], cpp_char("\n"));
}

func solve(x: dynamic) -> dynamic
{
  var k: dynamic = (x / 14);
  var r: dynamic = (x % 14);
  answer((((k > 0) && (r > 0)) && (r < 7)));
}

func test_case() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  read(x);
  solve(x);
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while ((cpp_update(t, "--") > 0))
  {
    test_case();
  }
  return 0;
}
