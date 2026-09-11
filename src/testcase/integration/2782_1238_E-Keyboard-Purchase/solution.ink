// Translated from solution.cpp.

var N: dynamic = 22;

var M: dynamic = 100005;

var INF: dynamic = 1000000009;

var m: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(M);

var q: dynamic = cpp_array(N, N);

var dp: dynamic = cpp_array((1 << N));

func main() -> dynamic
{
  scanf("%d%d", (&m), (&n));
  scanf(" %s", a);
  {
    var i: dynamic = 0;
    while ((i < (m - 1)))
    {
      q[(a[i] - cpp_char("a"))][(a[(i + 1)] - cpp_char("a"))] += 1;
      q[(a[(i + 1)] - cpp_char("a"))][(a[i] - cpp_char("a"))] += 1;
      i += 1;
    }
  }
  {
    var x: dynamic = 1;
    while ((x < ((1 << n))))
    {
      dp[x] = INF;
      x += 1;
    }
  }
  {
    var x: dynamic = 0;
    while ((x < (((1 << n)) - 1)))
    {
      var m: dynamic = 0;
      {
        var i: dynamic = 0;
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
        var i: dynamic = 0;
        while ((i < n))
        {
          if ((!((x & ((1 << i))))))
          {
            var q1: dynamic = 0;
            {
              var j: dynamic = 0;
              while ((j < n))
              {
                if (((x & ((1 << j)))))
                {
                  q1 += q[i][j];
                }
                j += 1;
              }
            }
            var q0: dynamic = 0;
            {
              var j: dynamic = 0;
              while ((j < n))
              {
                if (((!((x & ((1 << j))))) && (j != i)))
                {
                  q0 += q[i][j];
                }
                j += 1;
              }
            }
            var y: dynamic = ((x | ((1 << i))));
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
