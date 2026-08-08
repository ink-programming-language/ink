// Translated from solution.cpp.

var dp = cpp_array(20, 262149);

var kt = cpp_array(20);

var s = cpp_array(20, 20);

func solve(mask: dynamic, pr: dynamic, n: dynamic, t: dynamic, u: dynamic)
{
  if ((t == n))
  {
    return (cpp_cast(0));
  }
  if ((dp[mask][pr] != -1))
  {
    return dp[mask][pr];
  }
  dp[mask][pr] = 0;
  var nmask: dynamic;
  {
    var i = 0;
    while ((i < u))
    {
      if ((((mask & ((1 << i)))) == 0))
      {
        nmask = (mask + ((1 << i)));
        dp[mask][pr] = max(dp[mask][pr], ((s[pr][i] + kt[i]) + solve(nmask, i, n, (t + 1), u)));
      }
      i += 1;
    }
  }
  return dp[mask][pr];
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t = 1;
  while (cpp_update(t, "--"))
  {
    memset(dp, -1, cpp_sizeof((dp)));
    var n: dynamic;
    var m: dynamic;
    var k: dynamic;
    var x: dynamic;
    var y: dynamic;
    var c: dynamic;
    read(n, m, k);
    {
      var i = 0;
      while ((i < n))
      {
        read(kt[i]);
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= k))
      {
        read(x, y, c);
        s[(x - 1)][(y - 1)] = c;
        i += 1;
      }
    }
    var ans = INT_MIN;
    var nm = 0;
    {
      var i = 0;
      while ((i < n))
      {
        ans = max(ans, (kt[i] + solve((nm + ((1 << i))), i, m, 1, n)));
        i += 1;
      }
    }
    write(ans, "\n");
  }
}
