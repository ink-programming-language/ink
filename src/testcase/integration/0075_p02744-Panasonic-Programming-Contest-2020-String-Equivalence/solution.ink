// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(15);

func dfs(dep: dynamic, mx: dynamic) -> dynamic
{
  if ((dep == n))
  {
    puts(s);
    return;
  }
  {
    var i: dynamic = 0;
    while ((i < mx))
    {
      s[dep] = (i + cpp_char("a"));
      dfs((dep + 1), mx);
      i += 1;
    }
  }
  s[dep] = (mx + cpp_char("a"));
  dfs((dep + 1), (mx + 1));
}

func main() -> dynamic
{
  scanf("%d", (&n));
  dfs(0, 0);
  return 0;
}
