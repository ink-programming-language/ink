// Translated from solution.cpp.

var N: dynamic = (1e5 + 5);

var mod: dynamic = (1e9 + 7);

var dp: dynamic = cpp_array(1025, 64);

var ans: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var base: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(100);

var B: dynamic = cpp_array(N);

var L: dynamic = cpp_array(N);

var R: dynamic = cpp_array(N);

func dfs(cnt: dynamic, mask: dynamic, limit: dynamic, zero: dynamic) -> dynamic
{
  if ((((!zero) && (!limit)) && (dp[cnt][mask] != -1)))
  {
    return dp[cnt][mask];
  }
  if ((cnt == 0))
  {
    return (((mask == 0)) && ((!zero)));
  }
  var res: dynamic = 0;
  if (limit)
  {
    if (zero)
    {
      {
        var i: dynamic = 1;
        while ((i < a[cnt]))
        {
          res += dfs((cnt - 1), (mask ^ ((1 << i))), 0, 0);
          i += 1;
        }
      }
      res += dfs((cnt - 1), (mask ^ ((1 << a[cnt]))), 1, 0);
      res += dfs((cnt - 1), mask, 0, 1);
    } else
    {
      {
        var i: dynamic = 0;
        while ((i < a[cnt]))
        {
          res += dfs((cnt - 1), (mask ^ ((1 << i))), 0, 0);
          i += 1;
        }
      }
      res += dfs((cnt - 1), (mask ^ ((1 << a[cnt]))), 1, 0);
    }
  } else
  {
    if (zero)
    {
      {
        var i: dynamic = 1;
        while ((i < base))
        {
          res += dfs((cnt - 1), (mask ^ ((1 << i))), 0, 0);
          i += 1;
        }
      }
      res += dfs((cnt - 1), mask, 0, 1);
    } else
    {
      {
        var i: dynamic = 0;
        while ((i < base))
        {
          res += dfs((cnt - 1), (mask ^ ((1 << i))), 0, 0);
          i += 1;
        }
      }
    }
  }
  if (((!limit) && (!zero)))
  {
    dp[cnt][mask] = res;
  }
  return res;
}

func cal(x: dynamic) -> dynamic
{
  if ((x <= 0))
  {
    return 0;
  }
  t = 0;
  while (x)
  {
    a[cpp_update(t, "++")] = (x % base);
    x /= base;
  }
  return dfs(t, 0, 1, 1);
}

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d %I64d %I64d", (B + i), (L + i), (R + i));
      i += 1;
    }
  }
  {
    var i: dynamic = 2;
    while ((i <= 10))
    {
      base = i;
      {
        var j: dynamic = 0;
        while ((j < 64))
        {
          {
            var k: dynamic = 0;
            while ((k < (1 << i)))
            {
              dp[j][k] = -1;
              k += 1;
            }
          }
          j += 1;
        }
      }
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          if ((B[j] != i))
          {
            j += 1;
            continue;
          }
          ans[j] = (cal(R[j]) - cal((L[j] - 1)));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      printf("%I64d\n", ans[i]);
      i += 1;
    }
  }
}
