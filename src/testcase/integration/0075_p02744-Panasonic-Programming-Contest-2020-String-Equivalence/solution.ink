// Translated from solution.cpp.

var n: dynamic;

var s = cpp_array(15);

func dfs(dep: dynamic, mx: dynamic)
{
  if ((dep == n))
  {
    puts(s);
    return;
  }
  {
    var i = 0;
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

func main()
{
  scanf("%d", (&n));
  dfs(0, 0);
  return 0;
}
