// Translated from solution.cpp.

var maxn = 501;

var mod = 1000000007;

var ool = (1e18 + 7);

var o = 1;

var a = cpp_array(maxn);

var b = cpp_array(maxn);

var k = cpp_array(maxn);

var n: dynamic;

var minz: dynamic;

var cx = cpp_array(maxn);

var cy = cpp_array(maxn);

var px = cpp_array(maxn);

var py = cpp_array(maxn);

var we = cpp_array(maxn, maxn);

var visx = cpp_array(maxn);

var visy = cpp_array(maxn);

var slack = cpp_array(maxn);

func dfs(u: dynamic)
{
  visx[u] = 1;
  {
    var v = (0);
    while ((v < (n)))
    {
      if (visy[v])
      {
        v += 1;
        continue;
      }
      var t = ((cx[u] + cy[v]) - we[u][v]);
      if ((t == 0))
      {
        visy[v] = 1;
        if (((py[v] == -1) || dfs(py[v])))
        {
          py[v] = u;
          px[u] = v;
          return 1;
        }
      } else if ((t < slack[v]))
      {
        slack[v] = t;
      }
      v += 1;
    }
  }
  return 0;
}

func main(argument_0: dynamic)
{
  scanf("%lld", (&n));
  {
    var i = (0);
    while ((i < (n)))
    {
      scanf("%lld%lld%lld", (&a[i]), (&b[i]), (&k[i]));
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < (n)))
    {
      px[i] = cpp_assign(py[i], "=", -1);
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < (n)))
    {
      {
        var j = (0);
        while ((j < (n)))
        {
          we[i][j] = max(0, (a[j] - (b[j] * min(k[j], cpp_cast(i)))));
          cx[i] = max(cx[i], we[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < (n)))
    {
      {
        var i = 0;
        while ((i < n))
        {
          slack[i] = ool;
          i += 1;
        }
      }
      {
        while (true)
        {
          memset(visx, false, cpp_sizeof((visx)));
          memset(visy, false, cpp_sizeof((visy)));
          if (dfs(i))
          {
            break;
          }
          var minz = ool;
          {
            var i = (0);
            while ((i < (n)))
            {
              if ((!visy[i]))
              {
                minz = min(minz, slack[i]);
              }
              i += 1;
            }
          }
          {
            var i = (0);
            while ((i < (n)))
            {
              if (visx[i])
              {
                cx[i] -= minz;
              }
              if (visy[i])
              {
                cy[i] += minz;
              } else
              {
                slack[i] -= minz;
              }
              i += 1;
            }
          }
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = (0);
    while ((i < (n)))
    {
      if ((py[i] != -1))
      {
        ans += we[py[i]][i];
      }
      i += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
