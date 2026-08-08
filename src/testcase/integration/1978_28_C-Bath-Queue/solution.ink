// Translated from solution.cpp.

var N = 55;

var n: dynamic;

var m: dynamic;

var a = cpp_array(N);

var dp = cpp_array(N, N, N);

var C = cpp_array(N, N);

func qmod(a: dynamic, b: dynamic)
{
  var res = 1;
  while (b)
  {
    if ((b & 1))
    {
      res = (res * a);
    }
    a = (a * a);
    b >>= 1;
  }
  return res;
}

func main()
{
  {
    var i = 0;
    while ((i < int_cpp(N)))
    {
      C[i][0] = 1;
      {
        var j = 1;
        while ((j < int_cpp((i + 1))))
        {
          C[i][j] = (C[(i - 1)][(j - 1)] + C[(i - 1)][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i < int_cpp((m + 1))))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < int_cpp((n + 1))))
    {
      dp[0][0][i] = i;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < int_cpp((m + 1))))
    {
      {
        var j = 0;
        while ((j < int_cpp((n + 1))))
        {
          {
            var k = 0;
            while ((k < int_cpp((n + 1))))
            {
              {
                var c = 0;
                while ((c < int_cpp((j + 1))))
                {
                  var p = max(k, ((((c + a[i]) - 1)) / a[i]));
                  dp[i][j][k] += (((dp[(i - 1)][(j - c)][p] * C[j][c]) * qmod((i - 1), (j - c))) / qmod(i, j));
                  c += 1;
                }
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
  printf("%.12lf\n", dp[m][n][0]);
  return 0;
}
