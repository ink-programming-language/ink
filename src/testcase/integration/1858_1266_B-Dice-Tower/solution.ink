// Translated from solution.cpp.

func answer(v: dynamic)
{
  var s = ["NO", "YES"];
  write(s[v], cpp_char("\n"));
}

func solve(x: dynamic)
{
  var k = (x / 14);
  var r = (x % 14);
  answer((((k > 0) && (r > 0)) && (r < 7)));
}

func test_case()
{
  var x: dynamic;
  read(x);
  solve(x);
}

func main()
{
  var t: dynamic;
  read(t);
  while ((cpp_update(t, "--") > 0))
  {
    test_case();
  }
  return 0;
}
