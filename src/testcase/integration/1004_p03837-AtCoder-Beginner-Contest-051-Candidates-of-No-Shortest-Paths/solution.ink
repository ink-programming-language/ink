// Translated from solution.cpp.

var INF: dynamic = 1000000;

func main() -> dynamic
{
  var dp: dynamic = [];
  {
    var i: dynamic = 0;
    while ((i < 101))
    {
      {
        var j: dynamic = 0;
        while ((j < 101))
        {
          dp[i][j] = INF;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array(1001);
  var b: dynamic = cpp_array(1001);
  var c: dynamic = cpp_array(1001);
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(a[i], b[i], c[i]);
      dp[a[i]][b[i]] = c[i];
      dp[b[i]][a[i]] = c[i];
      i += 1;
    }
  }
  var flag: dynamic = true;
  while (flag)
  {
    flag = false;
    {
      var i: dynamic = 1;
      while ((i < (n + 1)))
      {
        {
          var j: dynamic = 1;
          while ((j < (n + 1)))
          {
            {
              var k: dynamic = 1;
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
  var r: dynamic = 0;
  {
    var i: dynamic = 0;
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
