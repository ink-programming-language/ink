// Translated from solution.cpp.

var N: dynamic = 55;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var dp: dynamic = cpp_array(N, N, N);

var C: dynamic = cpp_array(N, N);

func qmod(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = 1;
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

func main() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < int_cpp(N)))
    {
      C[i][0] = 1;
      {
        var j: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i < int_cpp((m + 1))))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < int_cpp((n + 1))))
    {
      dp[0][0][i] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < int_cpp((m + 1))))
    {
      {
        var j: dynamic = 0;
        while ((j < int_cpp((n + 1))))
        {
          {
            var k: dynamic = 0;
            while ((k < int_cpp((n + 1))))
            {
              {
                var c: dynamic = 0;
                while ((c < int_cpp((j + 1))))
                {
                  var p: dynamic = max(k, ((((c + a[i]) - 1)) / a[i]));
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
