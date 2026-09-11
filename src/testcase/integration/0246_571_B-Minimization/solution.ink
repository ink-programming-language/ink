// Translated from solution.cpp.

var inf: dynamic = 1E17;

var mod: dynamic = 1;

var a: dynamic = cpp_array(300010);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var chnk: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(5001, 5001);

func solve(pos: dynamic, xtra: dynamic, l: dynamic) -> dynamic
{
  if ((pos == 0))
  {
    if ((xtra == 0))
    {
      return 0;
    }
    return inf;
  }
  var ret: dynamic = dp[pos][xtra];
  if ((ret != -1))
  {
    return ret;
  }
  ret = ((a[((l + chnk) - 1)] - a[l]) + solve((pos - 1), xtra, (l + chnk)));
  if (xtra)
  {
    ret = min(ret, ((a[(l + chnk)] - a[l]) + solve((pos - 1), (xtra - 1), ((l + chnk) + 1))));
  }
  return ret;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  while (((cin >> n) >> k))
  {
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        read(a[i]);
        i += 1;
      }
    }
    sort((a + 1), ((a + n) + 1));
    memset(dp, -1, cpp_sizeof(dp));
    chnk = (n / k);
    write(solve(k, (n % k), 1), "\n");
  }
  return 0;
}
