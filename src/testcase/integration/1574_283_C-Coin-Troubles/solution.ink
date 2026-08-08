// Translated from solution.cpp.

var N = (5e5 + 5);

var mo = (1e9 + 7);

var n: dynamic;

var a = cpp_array(N);

var m: dynamic;

var q: dynamic;

var fa = cpp_array(N);

var vis = cpp_array(N);

var dep = cpp_array(N);

var ins = cpp_array(N);

var dp = cpp_array(N);

var sum = cpp_array(N);

var g = cpp_array(N);

func dfs(u: dynamic)
{
  ins[u] = 1;
  for (var v in g[u])
  {
    sum[v] = (a[v] + sum[u]);
    dfs(v);
    dep[u] = (dep[v] + 1);
  }
  m -= (dep[u] * a[u]);
}

func work()
{
  scanf("%lld%lld%lld", (&n), (&q), (&m));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lld", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    var u: dynamic;
    var v: dynamic;
    while ((i <= q))
    {
      scanf("%d%d", (&u), (&v));
      g[u].push_back(v);
      vis[v] = 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!vis[i]))
      {
        sum[i] = a[i];
        dfs(i);
      }
      i += 1;
    }
  }
  var fl = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      fl &= ins[i];
      i += 1;
    }
  }
  if (((m < 0) || (!fl)))
  {
    puts("0");
    return;
  }
  dp[0] = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = sum[i];
        while ((j <= m))
        {
          (cpp_assign(dp[j], "+=", dp[(j - sum[i])])) %= mo;
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%lld", dp[m]);
}

func main()
{
  FGF.work();
  return 0;
}
