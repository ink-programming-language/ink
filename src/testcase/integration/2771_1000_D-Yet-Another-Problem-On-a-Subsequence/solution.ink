// Translated from solution.cpp.

var maxn = (1e3 + 7);

var mod = 998244353;

var a = cpp_array(maxn);

var c = cpp_array(maxn, maxn);

var dp = cpp_array(maxn);

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      dp[i] = 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= n))
    {
      c[i][0] = 1;
      {
        var j = 1;
        while ((j <= i))
        {
          c[i][j] = (((c[(i - 1)][(j - 1)] + c[(i - 1)][j])) % mod);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((((i + a[i]) <= n) && (a[i] > 0)))
      {
        ans = (((ans + (dp[i] * c[(n - i)][a[i]]))) % mod);
        {
          var j = (i + a[i]);
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
