// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(110);

var b: dynamic = cpp_array(110);

var dp: dynamic = cpp_array(2, 101000, 110);

func Rec(i: dynamic, sum: dynamic, take: dynamic) -> dynamic
{
  if ((i == n))
  {
    return ( (((sum == 1e4) && take)) ? 0 : -1e9);
  }
  if ((dp[i][sum][take] != -1))
  {
    return dp[i][sum][take];
  }
  var Res: dynamic = -1e9;
  Res = max(Res, (a[i] + Rec((i + 1), (((sum + a[i])) - (b[i] * k)), 1)));
  Res = max(Res, Rec((i + 1), sum, take));
  return cpp_assign(dp[i][sum][take], "=", Res);
}

func main() -> dynamic
{
  memset(dp, -1, cpp_sizeof((dp)));
  read(n, k);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(b[i]);
      i += 1;
    }
  }
  write(( ((Rec(0, 1e4, 0) < 0)) ? -1 : Rec(0, 1e4, 0)));
  return 0;
}
