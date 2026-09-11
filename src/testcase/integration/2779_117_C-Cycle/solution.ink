// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_array(5005, 5005);

var vis: dynamic = cpp_array(5005);

func bfs(u: dynamic, pre: dynamic) -> dynamic
{
  vis[u] = 1;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((m[u][i] == cpp_char("1")))
      {
        if ((m[i][pre] - cpp_char("0")))
        {
          printf("%d %d %d\n", pre, u, i);
          return 1;
        }
        if (((!vis[i]) && bfs(i, u)))
        {
          return 1;
        }
      }
      i += 1;
    }
  }
  return 0;
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%s", (m[i] + 1));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (((!vis[i]) && bfs(i, i)))
      {
        return 0;
      }
      i += 1;
    }
  }
  puts("-1");
  return 0;
}
