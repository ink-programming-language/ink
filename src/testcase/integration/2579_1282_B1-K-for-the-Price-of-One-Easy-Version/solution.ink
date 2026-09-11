// Translated from solution.cpp.

var MOD: dynamic = (1e9 + 7);

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var p: dynamic = cpp_uninitialized();
    var k: dynamic = cpp_uninitialized();
    var i: dynamic = cpp_uninitialized();
    read(n, p, k);
    var a: dynamic = cpp_array(n);
    {
      i = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    sort(a, (a + n));
    var dp: dynamic = cpp_array(n);
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
    var ans: dynamic = 0;
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
