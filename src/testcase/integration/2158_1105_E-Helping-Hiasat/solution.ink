// Translated from solution.cpp.

var INF = 0x3f3f3f3f;

var N = 110;

var g = cpp_array(N, N);

var ans: dynamic;

var cnt = cpp_array(N);

var n: dynamic;

var m: dynamic;

var vis = cpp_array(N);

var M: dynamic;

var ppp: dynamic;

func dfs(u: dynamic, dep: dynamic)
{
  {
    var i = (u + 1);
    while ((i <= n))
    {
      var flag = 1;
      if (((cnt[i] + dep) <= ans))
      {
        return 0;
      }
      {
        var j = 0;
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

func run()
{
  ans = -1;
  {
    var i = n;
    while (i)
    {
      vis[0] = i;
      dfs(i, 1);
      cnt[i] = ans;
      i -= 1;
    }
  }
}

func main()
{
  scanf("%d%d", (&m), (&n));
  var Q: dynamic;
  memset(g, 1, cpp_sizeof(g));
  while (cpp_update(m, "--"))
  {
    var x: dynamic;
    scanf("%d", (&x));
    if ((x == 1))
    {
      Q.clear();
    } else
    {
      var str: dynamic;
      read(str);
      if ((!M[str]))
      {
        M[str] = cpp_update(ppp, "++");
      }
      {
        var i = 0;
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
