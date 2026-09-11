// Translated from solution.cpp.

var maxn: dynamic = 501;

var mod: dynamic = 1000000007;

var ool: dynamic = (1e18 + 7);

var o: dynamic = 1;

var a: dynamic = cpp_array(maxn);

var b: dynamic = cpp_array(maxn);

var k: dynamic = cpp_array(maxn);

var n: dynamic = cpp_uninitialized();

var minz: dynamic = cpp_uninitialized();

var cx: dynamic = cpp_array(maxn);

var cy: dynamic = cpp_array(maxn);

var px: dynamic = cpp_array(maxn);

var py: dynamic = cpp_array(maxn);

var we: dynamic = cpp_array(maxn, maxn);

var visx: dynamic = cpp_array(maxn);

var visy: dynamic = cpp_array(maxn);

var slack: dynamic = cpp_array(maxn);

func dfs(u: dynamic) -> dynamic
{
  visx[u] = 1;
  {
    var v: dynamic = (0);
    while ((v < (n)))
    {
      if (visy[v])
      {
        v += 1;
        continue;
      }
      var t: dynamic = ((cx[u] + cy[v]) - we[u][v]);
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

func main(argument_0: dynamic) -> dynamic
{
  scanf("%lld", (&n));
  {
    var i: dynamic = (0);
    while ((i < (n)))
    {
      scanf("%lld%lld%lld", (&a[i]), (&b[i]), (&k[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = (0);
    while ((i < (n)))
    {
      px[i] = cpp_assign(py[i], "=", -1);
      i += 1;
    }
  }
  {
    var i: dynamic = (0);
    while ((i < (n)))
    {
      {
        var j: dynamic = (0);
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
    var i: dynamic = (0);
    while ((i < (n)))
    {
      {
        var i: dynamic = 0;
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
          var minz: dynamic = ool;
          {
            var i: dynamic = (0);
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
            var i: dynamic = (0);
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
  var ans: dynamic = 0;
  {
    var i: dynamic = (0);
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
