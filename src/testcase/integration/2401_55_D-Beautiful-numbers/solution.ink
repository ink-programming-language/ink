// Translated from solution.cpp.

var a = cpp_array(20);

var p: dynamic;

var dp = cpp_array(50, 2525, 20);

var b = cpp_array(2525);

func gcd(m: dynamic, n: dynamic)
{
  var maxx = max(m, n);
  var minn = min(m, n);
  if ((minn == 0))
  {
    return maxx;
  }
  while (minn)
  {
    var x = minn;
    minn = (maxx % minn);
    maxx = x;
  }
  return ((m * n) / maxx);
}

var cnt = 0;

func dfs(pos: dynamic, pre: dynamic, GCD: dynamic, limit: dynamic)
{
  if ((b[GCD] == 0))
  {
    b[GCD] = cpp_update(cnt, "++");
  }
  if ((pos <= 0))
  {
    if ((GCD != 0))
    {
      if (((pre % GCD) == 0))
      {
        return 1;
      } else
      {
        return 0;
      }
    } else
    {
      return 0;
    }
  }
  if (((!limit) && (dp[pos][pre][b[GCD]] != -1)))
  {
    return dp[pos][pre][b[GCD]];
  }
  var n = if (limit) a[pos] else 9;
  var ans = 0;
  {
    var i = 0;
    while ((i <= n))
    {
      ans += dfs((pos - 1), ((((pre * 10) + i)) % 2520), gcd(i, GCD), (limit && (i == a[pos])));
      i += 1;
    }
  }
  if ((!limit))
  {
    dp[pos][pre][b[GCD]] = ans;
  }
  return ans;
}

func solve(x: dynamic)
{
  p = 1;
  memset(a, 0, cpp_sizeof((a)));
  while ((x > 0))
  {
    a[p] = (x % 10);
    x /= 10;
    p += 1;
  }
  return dfs((p - 1), 0, 0, 1);
}

func main()
{
  var T: dynamic;
  scanf("%d", (&T));
  var n: dynamic;
  var m: dynamic;
  memset(dp, -1, cpp_sizeof((dp)));
  while (cpp_update(T, "--"))
  {
    scanf("%I64d%I64d", (&n), (&m));
    printf("%I64d\n", (solve(m) - solve((n - 1))));
  }
  return 0;
}
