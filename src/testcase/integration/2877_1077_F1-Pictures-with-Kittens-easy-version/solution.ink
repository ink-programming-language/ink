// Translated from solution.cpp.

var dp: dynamic = cpp_array(5005, 5050);

var q: dynamic = cpp_array(2102100);

var a: dynamic = cpp_array(2100210);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, k, s);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      {
        var j: dynamic = 0;
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
    var j: dynamic = 1;
    while ((j <= s))
    {
      var l: dynamic = 0;
      var r: dynamic = 1;
      q[0] = 0;
      {
        var i: dynamic = 1;
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
  var maxx: dynamic = -1111111111111111;
  {
    var i: dynamic = ((n - k) + 1);
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
