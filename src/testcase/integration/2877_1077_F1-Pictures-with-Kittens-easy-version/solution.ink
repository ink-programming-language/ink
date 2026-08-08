// Translated from solution.cpp.

var dp = cpp_array(5005, 5050);

var q = cpp_array(2102100);

var a = cpp_array(2100210);

func main()
{
  var n: dynamic;
  var k: dynamic;
  var s: dynamic;
  read(n, k, s);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= n))
    {
      {
        var j = 0;
        while ((j <= s))
        {
          dp[i][j] = -1111111111111111;
          j += 1;
        }
      }
      i += 1;
    }
  }
  dp[0][0] = 0;
  {
    var j = 1;
    while ((j <= s))
    {
      var l = 0;
      var r = 1;
      q[0] = 0;
      {
        var i = 1;
        while ((i <= n))
        {
          while (((l < r) && (q[l] < (i - k))))
          {
            l += 1;
          }
          dp[i][j] = (dp[q[l]][(j - 1)] + a[i]);
          while (((l < r) && (dp[q[(r - 1)]][(j - 1)] <= dp[i][(j - 1)])))
          {
            r -= 1;
          }
          q[cpp_update(r, "++")] = i;
          i += 1;
        }
      }
      j += 1;
    }
  }
  var maxx = -1111111111111111;
  {
    var i = ((n - k) + 1);
    while ((i <= n))
    {
      maxx = max(maxx, dp[i][s]);
      i += 1;
    }
  }
  if ((maxx < 0))
  {
    write("-1", "\n");
    return 0;
  }
  write(maxx, "\n");
}
