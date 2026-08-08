// Translated from solution.cpp.

var INF = 1000000;

func main()
{
  var dp = [];
  {
    var i = 0;
    while ((i < 101))
    {
      {
        var j = 0;
        while ((j < 101))
        {
          dp[i][j] = INF;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var n: dynamic;
  var m: dynamic;
  var a = cpp_array(1001);
  var b = cpp_array(1001);
  var c = cpp_array(1001);
  read(n, m);
  {
    var i = 0;
    while ((i < m))
    {
      read(a[i], b[i], c[i]);
      dp[a[i]][b[i]] = c[i];
      dp[b[i]][a[i]] = c[i];
      i += 1;
    }
  }
  var flag = true;
  while (flag)
  {
    flag = false;
    {
      var i = 1;
      while ((i < (n + 1)))
      {
        {
          var j = 1;
          while ((j < (n + 1)))
          {
            {
              var k = 1;
              while ((k < (n + 1)))
              {
                if ((dp[i][j] > (dp[i][k] + dp[k][j])))
                {
                  flag = true;
                  dp[i][j] = (dp[i][k] + dp[k][j]);
                }
                k += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
  }
  var r = 0;
  {
    var i = 0;
    while ((i < m))
    {
      if ((c[i] > dp[a[i]][b[i]]))
      {
        r += 1;
      }
      i += 1;
    }
  }
  write(r, "\n");
  return 0;
}
