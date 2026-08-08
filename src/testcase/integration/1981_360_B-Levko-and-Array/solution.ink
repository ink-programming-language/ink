// Translated from solution.cpp.

func power(a: dynamic, b: dynamic, m: dynamic)
{
  var x = (1 % m);
  a %= m;
  while (b)
  {
    if (((1 & b)))
    {
      x = ((x * a) % m);
    }
    a = ((a * a) % m);
    b >>= 1;
  }
  return x;
}

var a = cpp_array(2005);

var n: dynamic;

var k: dynamic;

var dp = cpp_array(2005);

func solve(x: dynamic)
{
  {
    var i = 1;
    while ((i <= (n + 1)))
    {
      dp[i] = (i - 1);
      if ((i != (n + 1)))
      {
        {
          var j = (i - 1);
          while ((j > 0))
          {
            if ((abs((a[i] - a[j])) <= (x * ((i - j)))))
            {
              dp[i] = min(dp[i], (dp[j] + (((i - j) - 1))));
            }
            j -= 1;
          }
        }
      } else
      {
        {
          var j = (i - 1);
          while ((j > 0))
          {
            dp[i] = min(dp[i], (dp[j] + (((i - j) - 1))));
            j -= 1;
          }
        }
      }
      i += 1;
    }
  }
  return dp[(n + 1)];
}

func main()
{
  scanf("%d", (&n));
  scanf("%d", (&k));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  if ((n == 1))
  {
    printf("0");
    return 0;
  }
  var mx = 0;
  {
    var i = 2;
    while ((i <= n))
    {
      mx = max(mx, abs((a[i] - a[(i - 1)])));
      i += 1;
    }
  }
  var l = 0;
  var r = mx;
  while ((l < r))
  {
    var mid = (((l + r)) >> 1);
    if ((solve(mid) <= k))
    {
      r = mid;
    } else
    {
      l = (mid + 1);
    }
  }
  printf("%lld\n", l);
}
