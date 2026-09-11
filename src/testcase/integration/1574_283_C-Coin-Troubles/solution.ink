// Translated from solution.cpp.

var N: dynamic = (5e5 + 5);

var mo: dynamic = (1e9 + 7);

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var m: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var fa: dynamic = cpp_array(N);

var vis: dynamic = cpp_array(N);

var dep: dynamic = cpp_array(N);

var ins: dynamic = cpp_array(N);

var dp: dynamic = cpp_array(N);

var sum: dynamic = cpp_array(N);

var g: dynamic = cpp_array(N);

func dfs(u: dynamic) -> dynamic
{
  ins[u] = 1;
  for (var v: dynamic in g[u])
  {
    sum[v] = (a[v] + sum[u]);
    dfs(v);
    dep[u] = (dep[v] + 1);
  }
  m -= (dep[u] * a[u]);
}

func work() -> dynamic
{
  scanf("%lld%lld%lld", (&n), (&q), (&m));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lld", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    var u: dynamic = cpp_uninitialized();
    var v: dynamic = cpp_uninitialized();
    while ((i <= q))
    {
      scanf("%d%d", (&u), (&v));
      g[u].push_back(v);
      vis[v] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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
  var fl: dynamic = 1;
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = sum[i];
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

func main() -> dynamic
{
  FGF.work();
  return 0;
}
