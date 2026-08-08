// Translated from solution.cpp.

var dp = cpp_array(100005);

var ans = cpp_array(100005);

func solve(num: dynamic)
{
  if ((dp[num] != -1))
  {
    return;
  }
  ans[num] = -1;
  dp[num] = 0;
  var i: dynamic;
  var n = 2;
  var a: dynamic;
  var sum: dynamic;
  var myset: dynamic;
  myset.clear();
  while ((((2 * num) - (n * ((n - 1)))) > 0))
  {
    if ((((((2 * num) - (n * ((n - 1))))) % ((2 * n))) == 0))
    {
      a = ((((2 * num) - (n * ((n - 1))))) / ((2 * n)));
      {
        i = a;
        sum = 0;
        while ((i <= ((a + n) - 1)))
        {
          solve(i);
          sum ^= dp[i];
          i += 1;
        }
      }
      if (((sum == 0) && (ans[num] == -1)))
      {
        ans[num] = n;
      }
      myset.insert(sum);
    }
    n += 1;
  }
  while (myset.count(dp[num]))
  {
    dp[num] += 1;
  }
  return;
}

func main()
{
  var n: dynamic;
  while ((scanf("%d", (&n)) != EOF))
  {
    memset(dp, -1, cpp_sizeof((dp)));
    solve(n);
    printf("%d\n", ans[n]);
  }
  return 0;
}
