// Translated from solution.cpp.

var N = 30;

var mod = (int_cpp(1e9) + 7);

var eps = 1e-9;

var k: dynamic;

var n: dynamic;

var p = cpp_array(N);

var dp = cpp_array((1 << 21));

var a = cpp_array(N);

func main()
{
  scanf("%d%d", (&n), (&k));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%lf", (&p[i]));
      i += 1;
    }
  }
  dp[0] = 1;
  {
    var mask = 0;
    while ((mask < ((1 << n))))
    {
      var sum = 0;
      {
        var i = 0;
        while ((i < n))
        {
          if ((mask & ((1 << i))))
          {
            sum += p[i];
          }
          i += 1;
        }
      }
      if (((builtin_popcount(mask) == k) || (abs((sum - 1)) < eps)))
      {
        {
          var i = 0;
          while ((i < n))
          {
            if ((mask & ((1 << i))))
            {
              a[i] += dp[mask];
            }
            i += 1;
          }
        }
        mask += 1;
        continue;
      }
      {
        var i = 0;
        while ((i < n))
        {
          if ((!((mask & ((1 << i))))))
          {
            var nmask = (mask | ((1 << i)));
            dp[nmask] += ((dp[mask] * p[i]) / ((1 - sum)));
          }
          i += 1;
        }
      }
      mask += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      printf("%.12lf ", double(a[i]));
      i += 1;
    }
  }
  return 0;
}
