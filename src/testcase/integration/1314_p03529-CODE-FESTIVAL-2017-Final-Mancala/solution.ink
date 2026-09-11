// Translated from solution.cpp.

var mod: dynamic = 1000000007;

var inf: dynamic = (mod * mod);

var d2: dynamic = 500000004;

var EPS: dynamic = 1e-6;

var PI: dynamic = acos(-1.0);

func ABS(a: dynamic) -> dynamic
{
  return max(a, (-a));
}

func ABS(a: dynamic) -> dynamic
{
  return max(a, (-a));
}

var dp: dynamic = cpp_array(11000, 110);

var inv: dynamic = cpp_array(110000);

var K: dynamic = cpp_uninitialized();

func calc(a: dynamic, b: dynamic) -> dynamic
{
  if ((dp[a][b] >= 0))
  {
    return dp[a][b];
  }
  if ((a == 0))
  {
    return cpp_assign(dp[a][b], "=", 0);
  }
  var ret: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= K))
    {
      var f: dynamic = (((b + i)) / a);
      var h: dynamic = (((b + i)) % a);
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

func main() -> dynamic
{
  inv[1] = 1;
  {
    var i: dynamic = 2;
    while ((i < 110000))
    {
      inv[i] = (((mod - ((((mod / i)) * inv[(mod % i)]) % mod))) % mod);
      i += 1;
    }
  }
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  scanf("%d%d", (&a), (&b));
  K = b;
  {
    var i: dynamic = 0;
    while ((i < 110))
    {
      {
        var j: dynamic = 0;
        while ((j < 11000))
        {
          dp[i][j] = -1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ret: dynamic = calc(a, 0);
  {
    var i: dynamic = 0;
    while ((i < a))
    {
      ret = ((ret * ((b + 1))) % mod);
      i += 1;
    }
  }
  printf("%lld\n", ret);
}
