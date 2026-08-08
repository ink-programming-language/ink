// Translated from solution.cpp.

var mod = 1000000007;

var inf = (mod * mod);

var d2 = 500000004;

var EPS = 1e-6;

var PI = acos(-1.0);

func ABS(a: dynamic)
{
  return max(a, (-a));
}

func ABS(a: dynamic)
{
  return max(a, (-a));
}

var dp = cpp_array(11000, 110);

var inv = cpp_array(110000);

var K: dynamic;

func calc(a: dynamic, b: dynamic)
{
  if ((dp[a][b] >= 0))
  {
    return dp[a][b];
  }
  if ((a == 0))
  {
    return cpp_assign(dp[a][b], "=", 0);
  }
  var ret = 0;
  {
    var i = 0;
    while ((i <= K))
    {
      var f = (((b + i)) / a);
      var h = (((b + i)) % a);
      if ((i > a))
      {
        f = 0;
        h = (b + i);
      }
      ret = (((ret + (inv[(K + 1)] * ((calc((a - 1), (b + f)) + h))))) % mod);
      i += 1;
    }
  }
  return cpp_assign(dp[a][b], "=", ret);
}

func main()
{
  inv[1] = 1;
  {
    var i = 2;
    while ((i < 110000))
    {
      inv[i] = (((mod - ((((mod / i)) * inv[(mod % i)]) % mod))) % mod);
      i += 1;
    }
  }
  var a: dynamic;
  var b: dynamic;
  scanf("%d%d", (&a), (&b));
  K = b;
  {
    var i = 0;
    while ((i < 110))
    {
      {
        var j = 0;
        while ((j < 11000))
        {
          dp[i][j] = -1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ret = calc(a, 0);
  {
    var i = 0;
    while ((i < a))
    {
      ret = ((ret * ((b + 1))) % mod);
      i += 1;
    }
  }
  printf("%lld\n", ret);
}
