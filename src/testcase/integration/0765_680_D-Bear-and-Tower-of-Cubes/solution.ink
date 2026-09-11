// Translated from solution.cpp.

var res: dynamic = cpp_uninitialized();

func my_pow(x: dynamic) -> dynamic
{
  return ((x * x) * x);
}

func dfs(m: dynamic, s: dynamic, v: dynamic) -> dynamic
{
  var x: dynamic = 0;
  if ((m <= 0))
  {
    res = max(res, make_pair(s, v));
    return;
  }
  while ((my_pow((x + 1)) <= m))
  {
    x += 1;
  }
  dfs((m - my_pow(x)), (s + 1), (v + my_pow(x)));
  if (((x - 1) > 0))
  {
    dfs(((my_pow(x) - 1) - my_pow((x - 1))), (s + 1), (v + my_pow((x - 1))));
  }
}

func main() -> dynamic
{
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var N: dynamic = 0;
  var X: dynamic = 0;
  var ansn: dynamic = 0;
  var ansX: dynamic = 0;
  var flag: dynamic = 0;
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  scanf("%I64d", (&m));
  dfs(m, 0, 0);
  printf("%I64d %I64d\n", res.first, res.second);
}
