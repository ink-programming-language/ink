// Translated from solution.cpp.

var INF: dynamic = 0x3f3f3f3f;

var N: dynamic = 110;

var g: dynamic = cpp_array(N, N);

var ans: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(N);

var M: dynamic = cpp_uninitialized();

var ppp: dynamic = cpp_uninitialized();

func dfs(u: dynamic, dep: dynamic) -> dynamic
{
  {
    var i: dynamic = (u + 1);
    while ((i <= n))
    {
      var flag: dynamic = 1;
      if (((cnt[i] + dep) <= ans))
      {
        return 0;
      }
      {
        var j: dynamic = 0;
        while ((j < dep))
        {
          if ((!g[i][vis[j]]))
          {
            flag = 0;
            break;
          }
          j += 1;
        }
      }
      if (flag)
      {
        vis[dep] = i;
        if (dfs(i, (dep + 1)))
        {
          return 1;
        }
      }
      i += 1;
    }
  }
  if ((dep > ans))
  {
    ans = dep;
    return 1;
  }
  return 0;
}

func run() -> dynamic
{
  ans = -1;
  {
    var i: dynamic = n;
    while (i)
    {
      vis[0] = i;
      dfs(i, 1);
      cnt[i] = ans;
      i -= 1;
    }
  }
}

func main() -> dynamic
{
  scanf("%d%d", (&m), (&n));
  var Q: dynamic = cpp_uninitialized();
  memset(g, 1, cpp_sizeof(g));
  while (cpp_update(m, "--"))
  {
    var x: dynamic = cpp_uninitialized();
    scanf("%d", (&x));
    if ((x == 1))
    {
      Q.clear();
    } else
    {
      var str: dynamic = cpp_uninitialized();
      read(str);
      if ((!M[str]))
      {
        M[str] = cpp_update(ppp, "++");
      }
      {
        var i: dynamic = 0;
        while ((i < Q.size()))
        {
          g[Q[i]][M[str]] = cpp_assign(g[M[str]][Q[i]], "=", 0);
          i += 1;
        }
      }
      Q.push_back(M[str]);
    }
  }
  run();
  printf("%d\n", ans);
  return 0;
}
