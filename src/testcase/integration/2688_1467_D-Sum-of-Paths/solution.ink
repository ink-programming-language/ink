// Translated from solution.cpp.

var N: dynamic = 5005;

var p: dynamic = (1e9 + 7);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var ans: dynamic = cpp_uninitialized();

var num: dynamic = cpp_array(N);

var dp: dynamic = cpp_array(N, N);

func main() -> dynamic
{
  scanf("%d%d%d", (&n), (&k), (&q));
  k += 1;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      {
        var t: dynamic = 1;
        while ((t <= n))
        {
          dp[t][i] = ( ((i == 1)) ? 1 : (((dp[(t - 1)][(i - 1)] + dp[(t + 1)][(i - 1)])) % p));
          t += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var t: dynamic = 1;
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
    var i: dynamic = 1;
    var id: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
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
