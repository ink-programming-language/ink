// Translated from solution.cpp.

var N = 5005;

var p = (1e9 + 7);

var n: dynamic;

var k: dynamic;

var q: dynamic;

var a = cpp_array(N);

var ans: dynamic;

var num = cpp_array(N);

var dp = cpp_array(N, N);

func main()
{
  scanf("%d%d%d", (&n), (&k), (&q));
  k += 1;
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= k))
    {
      {
        var t = 1;
        while ((t <= n))
        {
          dp[t][i] = (if ((i == 1)) 1 else (((dp[(t - 1)][(i - 1)] + dp[(t + 1)][(i - 1)])) % p));
          t += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var t = 1;
        while ((t <= k))
        {
          num[i] = (((num[i] + (dp[i][t] * dp[i][((k - t) + 1)]))) % p);
          t += 1;
        }
      }
      ans = (((ans + (a[i] * num[i]))) % p);
      i += 1;
    }
  }
  {
    var i = 1;
    var id: dynamic;
    var x: dynamic;
    while ((i <= q))
    {
      scanf("%d%d", (&id), (&x));
      ans = ((((ans + (((1 * ((x - a[id]))) * num[id]) % p)) + p)) % p);
      a[id] = x;
      printf("%lld\n", ans);
      i += 1;
    }
  }
  return 0;
}
