// Translated from solution.cpp.

var dp = cpp_array(65536, 100);

var c = cpp_array(16, 30);

var d = cpp_array(16, 30);

var n: dynamic;

var m: dynamic;

var p = cpp_array(16);

var q = cpp_array(16);

func count()
{
  var cnt2 = 0;
  {
    var i = 0;
    while ((i < 16))
    {
      cnt2 += (q[i] * ((1 << i)));
      i += 1;
    }
  }
  return cnt2;
}

func main()
{
  while (true)
  {
    {
      var i = 0;
      while ((i < 100))
      {
        {
          var j = 0;
          while ((j < 65536))
          {
            dp[i][j] = -1;
            c[(i % 30)][(j % 16)] = 0;
            d[(i % 30)][(j % 16)] = 0;
            j += 1;
          }
        }
        i += 1;
      }
    }
    read(n, m);
    var cnt = 0;
    if (((n == 0) && (m == 0)))
    {
      break;
    }
    {
      var i = 0;
      while ((i < n))
      {
        {
          var j = 0;
          while ((j < 16))
          {
            read(c[i][j]);
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < m))
      {
        {
          var j = 0;
          while ((j < 16))
          {
            read(d[i][j]);
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < n))
      {
        cnt += (c[0][i] * ((1 << i)));
        i += 1;
      }
    }
    dp[0][0] = 0;
    {
      var i = 0;
      while ((i < n))
      {
        {
          var j = 0;
          while ((j < 65536))
          {
            if ((dp[i][j] == -1))
            {
              j += 1;
              continue;
            }
            {
              var k = 0;
              while ((k < 16))
              {
                p[k] = (((j / ((1 << k)))) % 2);
                k += 1;
              }
            }
            {
              var k = 0;
              while ((k < 16))
              {
                if ((c[i][k] == 1))
                {
                  p[k] = 1;
                }
                k += 1;
              }
            }
            {
              var k = 0;
              while ((k < m))
              {
                {
                  var l = 0;
                  while ((l < 16))
                  {
                    q[l] = p[l];
                    l += 1;
                  }
                }
                cnt = 0;
                {
                  var l = 0;
                  while ((l < 16))
                  {
                    if ((d[k][l] == 1))
                    {
                      if ((q[l] == 1))
                      {
                        cnt += 1;
                      }
                      q[l] = 0;
                    }
                    l += 1;
                  }
                }
                dp[(i + 1)][count()] = max(dp[(i + 1)][count()], (dp[i][j] + cnt));
                k += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    var minx = 0;
    {
      var i = 0;
      while ((i < 65536))
      {
        minx = max(minx, dp[n][i]);
        i += 1;
      }
    }
    write(minx, "\n");
  }
}
