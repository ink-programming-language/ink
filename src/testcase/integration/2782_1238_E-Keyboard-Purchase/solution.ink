// Translated from solution.cpp.

var N = 22;

var M = 100005;

var INF = 1000000009;

var m: dynamic;

var n: dynamic;

var a = cpp_array(M);

var q = cpp_array(N, N);

var dp = cpp_array((1 << N));

func main()
{
  scanf("%d%d", (&m), (&n));
  scanf(" %s", a);
  {
    var i = 0;
    while ((i < (m - 1)))
    {
      q[(a[i] - cpp_char("a"))][(a[(i + 1)] - cpp_char("a"))] += 1;
      q[(a[(i + 1)] - cpp_char("a"))][(a[i] - cpp_char("a"))] += 1;
      i += 1;
    }
  }
  {
    var x = 1;
    while ((x < ((1 << n))))
    {
      dp[x] = INF;
      x += 1;
    }
  }
  {
    var x = 0;
    while ((x < (((1 << n)) - 1)))
    {
      var m = 0;
      {
        var i = 0;
        while ((i < n))
        {
          if (((x & ((1 << i)))))
          {
            m += 1;
          }
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i < n))
        {
          if ((!((x & ((1 << i))))))
          {
            var q1 = 0;
            {
              var j = 0;
              while ((j < n))
              {
                if (((x & ((1 << j)))))
                {
                  q1 += q[i][j];
                }
                j += 1;
              }
            }
            var q0 = 0;
            {
              var j = 0;
              while ((j < n))
              {
                if (((!((x & ((1 << j))))) && (j != i)))
                {
                  q0 += q[i][j];
                }
                j += 1;
              }
            }
            var y = ((x | ((1 << i))));
            dp[y] = min(dp[y], ((dp[x] + (q1 * m)) - (q0 * m)));
          }
          i += 1;
        }
      }
      x += 1;
    }
  }
  write(dp[(((1 << n)) - 1)], "\n");
  return 0;
}
