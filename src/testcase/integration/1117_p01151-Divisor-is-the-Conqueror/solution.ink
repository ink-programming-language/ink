// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic, m: dynamic)
{
  cpp_macro("for(int i=n;i<m;i++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <");
}

var n: dynamic;

var cnt = cpp_array(14);

var ans = cpp_array(54);

func solve(idx: dynamic, sum: dynamic)
{
  if (((idx == 0) && (sum == 0)))
  {
    return true;
  }
  if (((idx < 0) || (sum < 0)))
  {
    return false;
  }
  rep(i, 14);
  {
    if (((cnt[i] > 0) && ((((sum - i)) % i) == 0)))
    {
      ans[(idx - 1)] = i;
      cnt[i] -= 1;
      if (solve((idx - 1), (sum - i)))
      {
        return true;
      }
      cnt[i] += 1;
    }
  }
  return false;
}

func main(argument_0: dynamic)
{
  while (cpp_comma((cin >> n), n))
  {
    var sum = 0;
    memset(cnt, 0, cpp_sizeof((cnt)));
    var flg = solve(n, sum);
    if (flg)
    {
      ((rep(i, (n - 1)) << ans[i]) << " ");
      write(ans[(n - 1)], "\n");
    } else
    {
      write("No\n");
    }
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var x: dynamic;
      read(x);
      sum += x;
      cnt[x] += 1;
    }
