// Translated from solution.cpp.

var INF: dynamic = 1e18;

var lim: dynamic = 200000;

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var dp: dynamic = cpp_construct((lim + 1), 0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var ndp: dynamic = cpp_construct((lim + 1));
      {
        var j: dynamic = 0;
        while ((j <= lim))
        {
          ndp[j] = max(dp[j], (fabs((a[i] - j)) / a[i]));
          j += 1;
        }
      }
      {
        var j: dynamic = 1;
        while ((j <= lim))
        {
          {
            var k: dynamic = (j * 2);
            while ((k <= lim))
            {
              ndp[k] = min(ndp[k], ndp[j]);
              k += j;
            }
          }
          j += 1;
        }
      }
      dp = ndp;
      i += 1;
    }
  }
  var ans: dynamic = INF;
  {
    var i: dynamic = 0;
    while ((i <= lim))
    {
      ans = min(ans, dp[i]);
      i += 1;
    }
  }
  write(fixed, setprecision(10));
  write(ans, "\n");
  return 0;
}
