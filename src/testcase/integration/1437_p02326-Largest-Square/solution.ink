// Translated from solution.cpp.

var maxn = 1400;

var dp = cpp_array(maxn, maxn);

var g = cpp_array(maxn, maxn);

func getlargestsquare(h: dynamic, w: dynamic)
{
  var maxwidth = 0;
  {
    var i = 0;
    while ((i < h))
    {
      {
        var j = 0;
        while ((j < w))
        {
          dp[i][j] = (((g[i][j] + 1)) % 2);
          maxwidth |= dp[i][j];
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < h))
    {
      {
        var j = 1;
        while ((j < w))
        {
          if (g[i][j])
          {
            dp[i][j] = 0;
          } else
          {
            dp[i][j] = (min(dp[(i - 1)][(j - 1)], min(dp[(i - 1)][j], dp[i][(j - 1)])) + 1);
            maxwidth = max(maxwidth, dp[i][j]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return (maxwidth * maxwidth);
}

func main()
{
  var h: dynamic;
  var w: dynamic;
  read(h, w);
  {
    var i = 0;
    while ((i < h))
    {
      {
        var j = 0;
        while ((j < w))
        {
          read(g[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", getlargestsquare(h, w));
  return 0;
}
