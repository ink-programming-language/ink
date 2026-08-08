// Translated from solution.cpp.

var MOD = (1e9 + 7);

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var p: dynamic;
    var k: dynamic;
    var i: dynamic;
    read(n, p, k);
    var a = cpp_array(n);
    {
      i = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    sort(a, (a + n));
    var dp = cpp_array(n);
    {
      i = 0;
      while ((i < n))
      {
        if ((i >= k))
        {
          if (((i - k) < 0))
          {
            dp[i] = a[i];
          } else
          {
            dp[i] = (a[i] + dp[(i - k)]);
          }
        } else
        {
          if ((i == 0))
          {
            dp[i] = a[i];
          } else
          {
            dp[i] = a[i];
          }
        }
        i += 1;
      }
    }
    var ans = 0;
    {
      i = 0;
      while ((i < n))
      {
        if ((dp[i] <= p))
        {
          ans = (i + 1);
        }
        i += 1;
      }
    }
    write(ans, "\n");
  }
  return 0;
}
