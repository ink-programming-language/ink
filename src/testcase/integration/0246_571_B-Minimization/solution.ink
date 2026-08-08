// Translated from solution.cpp.

var inf = 1E17;

var mod = 1;

var a = cpp_array(300010);

var n: dynamic;

var k: dynamic;

var chnk: dynamic;

var dp = cpp_array(5001, 5001);

func solve(pos: dynamic, xtra: dynamic, l: dynamic)
{
  if ((pos == 0))
  {
    if ((xtra == 0))
    {
      return 0;
    }
    return inf;
  }
  var ret = dp[pos][xtra];
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

func main()
{
  ios_base.sync_with_stdio(false);
  while (((cin >> n) >> k))
  {
    {
      var i = 1;
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
