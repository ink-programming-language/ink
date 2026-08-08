// Translated from solution.cpp.

var s1: dynamic;

var s2: dynamic;

var n: dynamic;

var m: dynamic;

var dp = cpp_array(2, 11, 1001, 1001);

func solve(a: dynamic, b: dynamic, c: dynamic, d: dynamic)
{
  if ((((a == n) || (b == m)) || (c == 0)))
  {
    return 0;
  }
  if ((dp[a][b][c][d] != -1))
  {
    return dp[a][b][c][d];
  }
  var r = dp[a][b][c][d];
  if (d)
  {
    if ((s1[a] == s2[b]))
    {
      r = max(r, (1 + solve((a + 1), (b + 1), c, d)));
      r = max(r, (1 + solve((a + 1), (b + 1), (c - 1), d)));
    } else
    {
      r = max(r, solve(a, b, (c - 1), 0));
    }
  }
  if ((!d))
  {
    if ((s1[a] == s2[b]))
    {
      r = max(r, (1 + solve((a + 1), (b + 1), c, 1)));
    }
    r = max(r, solve((a + 1), b, c, 0));
    r = max(r, solve(a, (b + 1), c, 0));
  }
  return r;
}

func main()
{
  var k: dynamic;
  read(n, m, k, s1, s2);
  memset(dp, -1, cpp_sizeof(dp));
  write(solve(0, 0, k, 0));
  return 0;
}
