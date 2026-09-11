// Translated from solution.cpp.

var maxn: dynamic = (1e3 + 7);

var mod: dynamic = 998244353;

var a: dynamic = cpp_array(maxn);

var c: dynamic = cpp_array(maxn, maxn);

var dp: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      dp[i] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      c[i][0] = 1;
      {
        var j: dynamic = 1;
        while ((j <= i))
        {
          c[i][j] = (((c[(i - 1)][(j - 1)] + c[(i - 1)][j])) % mod);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((((i + a[i]) <= n) && (a[i] > 0)))
      {
        ans = (((ans + (dp[i] * c[(n - i)][a[i]]))) % mod);
        {
          var j: dynamic = (i + a[i]);
          while ((j <= n))
          {
            dp[(j + 1)] = (((dp[(j + 1)] + (dp[i] * c[(j - i)][a[i]]))) % mod);
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  write(ans);
  return 0;
}
