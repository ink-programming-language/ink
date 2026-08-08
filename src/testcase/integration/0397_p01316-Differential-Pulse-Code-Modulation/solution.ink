// Translated from solution.cpp.

var IF = cpp_expression("#include <");

func lengthof(x: dynamic)
{
  return cpp_expression("#include <iostream> #inclu");
}

func main(argc: dynamic, argv: dynamic)
{
  var n: dynamic;
  var m: dynamic;
  var dp = cpp_array(2, (1 << 8));
  var min: dynamic;
  while (1)
  {
    read(n, m);
    if (((n + m) == 0))
    {
      break;
    }
    {
      var i1 = 0;
      while ((i1 < m))
      {
        read(cb[i1]);
        i1 += 1;
      }
    }
    {
      var i1 = 0;
      while ((i1 < n))
      {
        read(x[i1]);
        i1 += 1;
      }
    }
    fill(cpp_cast(dp), cpp_cast(((dp + lengthof(dp)))), IF);
    dp[128][0] = 0;
    {
      var i1 = 0;
      while ((i1 < n))
      {
        {
          var i2 = 0;
          while ((i2 < ((1 << 8))))
          {
            if ((dp[i2][(i1 % 2)] != IF))
            {
              {
                var i3 = 0;
                while ((i3 < m))
                {
                  var temp = (i2 + cb[i3]);
                  if ((temp < 0))
                  {
                    temp = 0;
                  }
                  if ((temp > 255))
                  {
                    temp = 255;
                  }
                  dp[temp][(((i1 + 1)) % 2)] = min(dp[temp][(((i1 + 1)) % 2)], (dp[i2][(i1 % 2)] + (((temp - x[i1])) * ((temp - x[i1])))));
                  i3 += 1;
                }
              }
              dp[i2][(i1 % 2)] = IF;
            }
            i2 += 1;
          }
        }
        i1 += 1;
      }
    }
    min = IF;
    {
      var i1 = 0;
      while ((i1 < ((1 << 8))))
      {
        min = min(min, dp[i1][(n % 2)]);
        i1 += 1;
      }
    }
    write(min, "\n");
  }
  return 0;
}
