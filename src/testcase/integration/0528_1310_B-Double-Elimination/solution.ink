// Translated from solution.cpp.

func read()
{
  var x = 0;
  var f = 1;
  var c = getchar();
  {
    while (((c > cpp_char("9")) || (c < cpp_char("0"))))
    {
      if ((c == cpp_char("-")))
      {
        f = -1;
      }
      c = getchar();
    }
  }
  {
    while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
    {
      x = (((x * 10) + c) - cpp_char("0"));
      c = getchar();
    }
  }
  return (x * f);
}

var dp = cpp_array(2, 2, ((1 << 18)), 18);

var fan = cpp_array(200005);

func work()
{
  var n = read();
  var k = read();
  {
    var i = 1;
    while ((i <= k))
    {
      var x = read();
      fan[x] = 1;
      i += 1;
    }
  }
  memset(dp, -0x3f, cpp_sizeof(dp));
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= ((1 << n))))
        {
          if ((i == 1))
          {
            dp[i][j][fan[j]][fan[(j + 1)]] = ((fan[j] | fan[(j + 1)]));
            dp[i][j][fan[(j + 1)]][fan[j]] = ((fan[j] | fan[(j + 1)]));
          } else
          {
            {
              var x1 = 0;
              while ((x1 < 2))
              {
                {
                  var x2 = 0;
                  while ((x2 < 2))
                  {
                    {
                      var y1 = 0;
                      while ((y1 < 2))
                      {
                        {
                          var y2 = 0;
                          while ((y2 < 2))
                          {
                            var tmp = (dp[(i - 1)][j][x1][y1] + dp[(i - 1)][(j + ((1 << (i - 1))))][x2][y2]);
                            if ((x1 | x2))
                            {
                              tmp += 1;
                            }
                            if ((y1 | y2))
                            {
                              tmp += 1;
                            }
                            dp[i][j][x1][x2] = max(dp[i][j][x1][x2], (tmp + ((x2 | y1))));
                            dp[i][j][x1][x2] = max(dp[i][j][x1][x2], (tmp + ((x2 | y2))));
                            dp[i][j][x1][y1] = max(dp[i][j][x1][y1], (tmp + ((y1 | x2))));
                            dp[i][j][x1][y2] = max(dp[i][j][x1][y2], (tmp + ((y2 | x2))));
                            dp[i][j][x2][x1] = max(dp[i][j][x2][x1], (tmp + ((x1 | y1))));
                            dp[i][j][x2][x1] = max(dp[i][j][x2][x1], (tmp + ((x1 | y2))));
                            dp[i][j][x2][y1] = max(dp[i][j][x2][y1], (tmp + ((y1 | x1))));
                            dp[i][j][x2][y2] = max(dp[i][j][x2][y2], (tmp + ((y2 | x1))));
                            y2 += 1;
                          }
                        }
                        y1 += 1;
                      }
                    }
                    x2 += 1;
                  }
                }
                x1 += 1;
              }
            }
          }
          j += ((1 << i));
        }
      }
      i += 1;
    }
  }
  var ans = -0x3f3f3f3f;
  ans = max(ans, (dp[n][1][1][1] + 1));
  ans = max(ans, (dp[n][1][0][1] + 1));
  ans = max(ans, (dp[n][1][1][0] + 1));
  ans = max(ans, dp[n][1][0][0]);
  printf("%d\n", ans);
}

func main()
{
  work();
  return 0;
}
