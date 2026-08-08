// Translated from solution.cpp.

var INF = 0x3f3f3f3f;

var maxn = 21;

var maxs = (1 << 21);

var n: dynamic;

var m: dynamic;

var a = cpp_array(maxn, maxn);

var str = cpp_array(maxn, maxn);

var dp = cpp_array(maxs);

func lowzero(s: dynamic)
{
  {
    var i = 0;
    while ((i < maxn))
    {
      if ((!((s & ((1 << i))))))
      {
        return i;
      }
      i += 1;
    }
  }
  return (maxn - 1);
}

func main()
{
  while ((~scanf("%d%d", (&n), (&m))))
  {
    {
      var i = 0;
      while ((i < n))
      {
        scanf("%s", str[i]);
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < n))
      {
        {
          var j = 0;
          while ((j < m))
          {
            scanf("%d", (&a[i][j]));
            j += 1;
          }
        }
        i += 1;
      }
    }
    memset(dp, 0xff, cpp_sizeof((dp)));
    dp[0] = 0;
    var M = (1 << n);
    {
      var s = 0;
      while ((s < M))
      {
        if ((dp[s] == -1))
        {
          s += 1;
          continue;
        }
        var bit = lowzero(s);
        {
          var j = 0;
          while ((j < m))
          {
            if (((dp[(s | ((1 << bit)))] == -1) || (dp[(s | ((1 << bit)))] > (dp[s] + a[bit][j]))))
            {
              dp[(s | ((1 << bit)))] = (dp[s] + a[bit][j]);
            }
            var sum = 0;
            var bits = 0;
            var mw = 0;
            {
              var i = 0;
              while ((i < n))
              {
                if ((str[i][j] == str[bit][j]))
                {
                  sum += a[i][j];
                  mw = max(mw, a[i][j]);
                  bits |= (1 << i);
                }
                i += 1;
              }
            }
            if (((dp[(s | bits)] == -1) || (dp[(s | bits)] > ((dp[s] + sum) - mw))))
            {
              dp[(s | bits)] = ((dp[s] + sum) - mw);
            }
            j += 1;
          }
        }
        s += 1;
      }
    }
    printf("%d\n", dp[(M - 1)]);
  }
  return 0;
}
