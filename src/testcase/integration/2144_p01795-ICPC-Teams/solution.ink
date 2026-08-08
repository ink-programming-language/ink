// Translated from solution.cpp.

var N = 1000100;

var P = (1e9 + 9);

var X = cpp_expression("#incl");

var Y = cpp_expression("#inclu");

var a = cpp_array(18);

var b = cpp_array(18);

var c = cpp_array(18);

var dp = cpp_array(N);

var f: dynamic;

var sz: dynamic;

func main()
{
  memset(dp, -1, cpp_sizeof((dp)));
  var n: dynamic;
  var m: dynamic;
  var ans = 0;
  scanf("%d%d", (&n), (&m));
  dp[0] = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      dp[i] = ((((((1 * (((3 * i) - 1))) * (((3 * i) - 2))) / 2) % P) * dp[(i - 1)]) % P);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      scanf("%d%d%d", (&a[i]), (&b[i]), (&c[i]));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < ((1 << m))))
    {
      var coef = 1;
      var res = 1;
      var cnt = [];
      var ff = 0;
      f.clear();
      sz.clear();
      {
        var j = 0;
        while ((j < m))
        {
          if ((i & ((1 << j))))
          {
            if (c[j])
            {
              coef *= -1;
            }
            if ((!f.count(a[j])))
            {
              f[a[j]] = a[j];
              sz[a[j]] = 1;
            }
            if ((!f.count(b[j])))
            {
              f[b[j]] = b[j];
              sz[b[j]] = 1;
            }
            if ((f[a[j]] != f[b[j]]))
            {
              if (((sz[f[a[j]]] + sz[f[b[j]]]) > 3))
              {
                ff = 1;
              } else if ((sz[f[a[j]]] < sz[f[b[j]]]))
              {
                f[a[j]] = f[b[j]];
                sz[f[b[j]]] += 1;
              } else
              {
                f[b[j]] = f[a[j]];
                sz[f[a[j]]] += 1;
              }
            }
          } else if ((!c[j]))
          {
            coef = 0;
          }
          j += 1;
        }
      }
      if ((!ff))
      {
        {
          var it = f.begin();
          while ((it != f.end()))
          {
            if ((it->X == it->Y))
            {
              cnt[sz[it->X]] += 1;
            }
            it += 1;
          }
        }
        cnt[1] = (((3 * n) - (2 * cnt[2])) - (3 * cnt[3]));
        {
          var i = 0;
          while ((i < cnt[2]))
          {
            res = (((1 * res) * cnt[1]) % P);
            cnt[1] -= 1;
            i += 1;
          }
        }
        res = (((1 * res) * dp[(cnt[1] / 3)]) % P);
      } else
      {
        res = 0;
      }
      ans = (((ans + ((1 * res) * ((P + coef))))) % P);
      i += 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
