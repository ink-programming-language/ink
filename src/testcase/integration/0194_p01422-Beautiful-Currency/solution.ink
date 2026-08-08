// Translated from solution.cpp.

var INF = 1e18;

var lim = 200000;

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var dp = cpp_construct((lim + 1), 0);
  {
    var i = 0;
    while ((i < n))
    {
      var ndp = cpp_construct((lim + 1));
      {
        var j = 0;
        while ((j <= lim))
        {
          ndp[j] = max(dp[j], (fabs((a[i] - j)) / a[i]));
          j += 1;
        }
      }
      {
        var j = 1;
        while ((j <= lim))
        {
          {
            var k = (j * 2);
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
  var ans = INF;
  {
    var i = 0;
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
