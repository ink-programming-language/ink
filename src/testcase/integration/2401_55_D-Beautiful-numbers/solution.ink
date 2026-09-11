// Translated from solution.cpp.

var a: dynamic = cpp_array(20);

var p: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(50, 2525, 20);

var b: dynamic = cpp_array(2525);

func gcd(m: dynamic, n: dynamic) -> dynamic
{
  var maxx: dynamic = max(m, n);
  var minn: dynamic = min(m, n);
  if ((minn == 0))
  {
    return maxx;
  }
  while (minn)
  {
    var x: dynamic = minn;
    minn = (maxx % minn);
    maxx = x;
  }
  return ((m * n) / maxx);
}

var cnt: dynamic = 0;

func dfs(pos: dynamic, pre: dynamic, GCD: dynamic, limit: dynamic) -> dynamic
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
  var n: dynamic =  (limit) ? a[pos] : 9;
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
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

func solve(x: dynamic) -> dynamic
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

func main() -> dynamic
{
  var T: dynamic = cpp_uninitialized();
  scanf("%d", (&T));
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  memset(dp, -1, cpp_sizeof((dp)));
  while (cpp_update(T, "--"))
  {
    scanf("%I64d%I64d", (&n), (&m));
    printf("%I64d\n", (solve(m) - solve((n - 1))));
  }
  return 0;
}
