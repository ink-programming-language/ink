// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(1501);

var aa: dynamic = cpp_array(1501);

var z: dynamic = cpp_array(1501);

var f: dynamic = cpp_array(1501, 1501);

var g: dynamic = cpp_array(1501);

var st: dynamic = cpp_array(1501);

var hz: dynamic = cpp_array(1502);

func ef(p: dynamic) -> dynamic
{
  var l: dynamic = 1;
  var r: dynamic = st[0];
  while ((l != r))
  {
    var mid: dynamic = ((l + r) >> 1);
    if ((st[mid] <= p))
    {
      r = mid;
    } else
    {
      l = (mid + 1);
    }
  }
  return l;
}

func pd(w: dynamic) -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      z[i] = (z[(i - 1)] + ((a[i] <= w)));
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= s))
    {
      st[0] = 0;
      {
        var j: dynamic = n;
        while (j)
        {
          hz[j] = max(hz[(j + 1)], f[(i - 1)][j]);
          while ((st[0] && ((f[(i - 1)][st[st[0]]] + z[(st[st[0]] - 1)]) <= (f[(i - 1)][j] + z[(j - 1)]))))
          {
            st[0] -= 1;
          }
          st[cpp_update(st[0], "++")] = j;
          if (g[j])
          {
            var c: dynamic = ef(g[j]);
            f[i][j] = max(((f[(i - 1)][st[c]] + z[(st[c] - 1)]) - z[(j - 1)]), ((hz[(g[j] + 1)] + z[g[j]]) - z[(j - 1)]));
          }
          ans = max(ans, f[i][j]);
          j -= 1;
        }
      }
      i += 1;
    }
  }
  return (ans >= k);
}

func main() -> dynamic
{
  scanf("%d%d%d%d", (&n), (&m), (&s), (&k));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      aa[i] = a[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var l: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      scanf("%d%d", (&l), (&r));
      g[l] = max(g[l], r);
      i += 1;
    }
  }
  sort((aa + 1), ((aa + n) + 1));
  aa[0] = ((unique((aa + 1), ((aa + n) + 1)) - aa) - 1);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[i] = (lower_bound((aa + 1), ((aa + aa[0]) + 1), a[i]) - aa);
      i += 1;
    }
  }
  var l: dynamic = 1;
  var r: dynamic = (aa[0] + 1);
  while ((l != r))
  {
    var mid: dynamic = ((l + r) >> 1);
    if (pd(mid))
    {
      r = mid;
    } else
    {
      l = (mid + 1);
    }
  }
  printf("%d\n",  ((l == (aa[0] + 1))) ? -1 : aa[l]);
}
