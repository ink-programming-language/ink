// Translated from solution.cpp.

var dp: dynamic = cpp_array(20, 262149);

var kt: dynamic = cpp_array(20);

var s: dynamic = cpp_array(20, 20);

func solve(mask: dynamic, pr: dynamic, n: dynamic, t: dynamic, u: dynamic) -> dynamic
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
  var nmask: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    memset(dp, -1, cpp_sizeof((dp)));
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    var k: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    var c: dynamic = cpp_uninitialized();
    read(n, m, k);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(kt[i]);
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= k))
      {
        read(x, y, c);
        s[(x - 1)][(y - 1)] = c;
        i += 1;
      }
    }
    var ans: dynamic = INT_MIN;
    var nm: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        ans = max(ans, (kt[i] + solve((nm + ((1 << i))), i, m, 1, n)));
        i += 1;
      }
    }
    write(ans, "\n");
  }
}
