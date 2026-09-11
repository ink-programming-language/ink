// Translated from solution.cpp.

var INF: dynamic = 0x3f3f3f3f;

var maxn: dynamic = 21;

var maxs: dynamic = (1 << 21);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn, maxn);

var str: dynamic = cpp_array(maxn, maxn);

var dp: dynamic = cpp_array(maxs);

func lowzero(s: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
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

func main() -> dynamic
{
  while ((~scanf("%d%d", (&n), (&m))))
  {
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        scanf("%s", str[i]);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        {
          var j: dynamic = 0;
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
    var M: dynamic = (1 << n);
    {
      var s: dynamic = 0;
      while ((s < M))
      {
        if ((dp[s] == -1))
        {
          s += 1;
          continue;
        }
        var bit: dynamic = lowzero(s);
        {
          var j: dynamic = 0;
          while ((j < m))
          {
            if (((dp[(s | ((1 << bit)))] == -1) || (dp[(s | ((1 << bit)))] > (dp[s] + a[bit][j]))))
            {
              dp[(s | ((1 << bit)))] = (dp[s] + a[bit][j]);
            }
            var sum: dynamic = 0;
            var bits: dynamic = 0;
            var mw: dynamic = 0;
            {
              var i: dynamic = 0;
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
