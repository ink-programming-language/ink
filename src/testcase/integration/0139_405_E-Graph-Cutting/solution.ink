// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var visit: dynamic = cpp_array(111111);

var v: dynamic = cpp_array(111111);

var mark: dynamic = cpp_array(111111);

func dfs(t: dynamic) -> dynamic
{
  var v1: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < v[t].size()))
    {
      var u: dynamic = v[t][i];
      if ((visit[mark[t][i]] == 0))
      {
        visit[mark[t][i]] = 1;
        var w: dynamic = dfs(u);
        if ((w == 0))
        {
          v1.push_back(u);
        } else
        {
          printf("%d %d %d\n", t, u, w);
        }
      }
      i += 1;
    }
  }
  while ((v1.size() > 1))
  {
    printf("%d %d %d\n", v1[(v1.size() - 1)], t, v1[(v1.size() - 2)]);
    v1.pop_back();
    v1.pop_back();
  }
  if ((v1.size() == 1))
  {
    return v1[0];
  } else
  {
    return 0;
  }
}

func main() -> dynamic
{
  memset(visit, 0, cpp_sizeof((visit)));
  scanf("%d%d", (&n), (&m));
  var i: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < m))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      scanf("%d%d", (&a), (&b));
      v[a].push_back(b);
      v[b].push_back(a);
      mark[a].push_back(i);
      mark[b].push_back(i);
      i += 1;
    }
  }
  if ((m % 2))
  {
    printf("No solution\n");
  } else
  {
    {
      i = 1;
      while ((i <= n))
      {
        dfs(i);
        i += 1;
      }
    }
  }
}
