// Translated from solution.cpp.

var mod: dynamic = (1e9 + 7);

var maxn: dynamic = 210;

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var mp: dynamic = cpp_array(maxn);

var dp: dynamic = cpp_array(maxn, maxn);

var tmp: dynamic = cpp_array(maxn);

func dfs(rt: dynamic, f: dynamic) -> dynamic
{
  dp[rt][0] = 1;
  dp[rt][(k + 1)] = 1;
  for (var v: dynamic in mp[rt])
  {
    if ((v == f))
    {
      continue;
    }
    dfs(v, rt);
    memset(tmp, 0, cpp_sizeof((tmp)));
    {
      var i: dynamic = 0;
      while ((i <= ((k * 2) + 1)))
      {
        {
          var j: dynamic = 0;
          while ((j <= (2 * k)))
          {
            if ((((j + i) + 1) <= ((2 * k) + 1)))
            {
              tmp[min(i, (j + 1))] += (((1 * dp[rt][i]) * dp[v][j]) % mod);
              tmp[min(i, (j + 1))] %= mod;
            } else
            {
              tmp[max(i, (j + 1))] += (((1 * dp[rt][i]) * dp[v][j]) % mod);
              tmp[max(i, (j + 1))] %= mod;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i <= ((2 * k) + 1)))
      {
        dp[rt][i] = tmp[i];
        i += 1;
      }
    }
  }
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&k));
  {
    var i: dynamic = 1;
    while ((i <= (n - 1)))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      scanf("%d%d", (&x), (&y));
      mp[x].push_back(y);
      mp[y].push_back(x);
      i += 1;
    }
  }
  dfs(1, 0);
  var res: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= k))
    {
      res += dp[1][i];
      i += 1;
    }
  }
  printf("%lld", (res % mod));
  return 0;
}
