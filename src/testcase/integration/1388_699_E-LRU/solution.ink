// Translated from solution.cpp.

var N: dynamic = 30;

var mod: dynamic = (int_cpp(1e9) + 7);

var eps: dynamic = 1e-9;

var k: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(N);

var dp: dynamic = cpp_array((1 << 21));

var a: dynamic = cpp_array(N);

func main() -> dynamic
{
  scanf("%d%d", (&n), (&k));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%lf", (&p[i]));
      i += 1;
    }
  }
  dp[0] = 1;
  {
    var mask: dynamic = 0;
    while ((mask < ((1 << n))))
    {
      var sum: dynamic = 0;
      {
        var i: dynamic = 0;
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
          var i: dynamic = 0;
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
        var i: dynamic = 0;
        while ((i < n))
        {
          if ((!((mask & ((1 << i))))))
          {
            var nmask: dynamic = (mask | ((1 << i)));
            dp[nmask] += ((dp[mask] * p[i]) / ((1 - sum)));
          }
          i += 1;
        }
      }
      mask += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      printf("%.12lf ", cpp_double(a[i]));
      i += 1;
    }
  }
  return 0;
}
