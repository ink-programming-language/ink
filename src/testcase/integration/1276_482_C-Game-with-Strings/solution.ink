// Translated from solution.cpp.

var inf = (1e9 + 333);

var linf = (1e18 + 333);

var N = 50;

var M = 21;

var n: dynamic;

var m: dynamic;

var cnt = cpp_array((1 << M));

var w = cpp_array((1 << M));

var dp = cpp_array((1 << M));

var s = cpp_array(M, N);

func main()
{
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%s", s[i]);
      i += 1;
    }
  }
  m = strlen(s[0]);
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < i))
        {
          var mask = 0;
          {
            var k = 0;
            while ((k < m))
            {
              mask |= (((s[i][k] == s[j][k])) << k);
              k += 1;
            }
          }
          w[mask] |= ((1 << i) | (1 << j));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = (((1 << m)) - 1);
    while ((i >= 0))
    {
      {
        var j = 0;
        while ((j < m))
        {
          if ((i & (1 << j)))
          {
            w[(i ^ (1 << j))] |= w[i];
          }
          j += 1;
        }
      }
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i < ((1 << m))))
    {
      cnt[i] = builtin_popcountll(w[i]);
      i += 1;
    }
  }
  {
    var i = (((1 << m)) - 1);
    while ((i >= 0))
    {
      if ((!cnt[i]))
      {
        i -= 1;
        continue;
      }
      var sum = 0;
      {
        var j = 0;
        while ((j < m))
        {
          if (((~i) & (1 << j)))
          {
            sum += (dp[(i | (1 << j))] * cnt[(i | (1 << j))]);
          }
          j += 1;
        }
      }
      dp[i] = (1 + ((sum / ((m - builtin_popcount(i)))) / cnt[i]));
      i -= 1;
    }
  }
  printf("%.18lf\n", dp[0]);
  return 0;
}
