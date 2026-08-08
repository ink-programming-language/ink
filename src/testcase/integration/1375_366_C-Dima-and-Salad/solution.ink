// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

var a = cpp_array(110);

var b = cpp_array(110);

var dp = cpp_array(2, 101000, 110);

func Rec(i: dynamic, sum: dynamic, take: dynamic)
{
  if ((i == n))
  {
    return (if (((sum == 1e4) && take)) 0 else -1e9);
  }
  if ((dp[i][sum][take] != -1))
  {
    return dp[i][sum][take];
  }
  var Res = -1e9;
  Res = max(Res, (a[i] + Rec((i + 1), (((sum + a[i])) - (b[i] * k)), 1)));
  Res = max(Res, Rec((i + 1), sum, take));
  return cpp_assign(dp[i][sum][take], "=", Res);
}

func main()
{
  memset(dp, -1, cpp_sizeof((dp)));
  read(n, k);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(b[i]);
      i += 1;
    }
  }
  write((if ((Rec(0, 1e4, 0) < 0)) -1 else Rec(0, 1e4, 0)));
  return 0;
}
