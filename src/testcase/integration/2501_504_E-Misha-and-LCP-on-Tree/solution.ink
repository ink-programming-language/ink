// Translated from solution.cpp.

var n: dynamic;

var head = cpp_array(300300);

var nxt = cpp_array((300300 << 1));

var to = cpp_array((300300 << 1));

var fa = cpp_array(22, 300300);

var dp = cpp_array(300300);

func add_edge(x: dynamic, y: dynamic, id: dynamic)
{
  nxt[id] = head[x];
  head[x] = id;
  to[id] = y;
}

func dfs(x: dynamic, p: dynamic = 0)
{
  fa[x][0] = p;
  {
    var i = 1;
    while ((i <= (31 - builtin_clz(n))))
    {
      fa[x][i] = fa[fa[x][(i - 1)]][(i - 1)];
      i += 1;
    }
  }
  {
    var i = head[x];
    while (i)
    {
      var y = to[i];
      if ((y == p))
      {
        i = nxt[i];
        continue;
      }
      dp[y] = (dp[x] + 1);
      dfs(y, x);
      i = nxt[i];
    }
  }
}

func anc(x: dynamic, d: dynamic)
{
  if ((!d))
  {
    return x;
  }
  {
    var i = (31 - builtin_clz(d));
    while ((i >= 0))
    {
      if ((d & ((1 << i))))
      {
        x = fa[x][i];
      }
      i -= 1;
    }
  }
  return x;
}

func lca(x: dynamic, y: dynamic)
{
  if ((dp[x] > dp[y]))
  {
    x = anc(x, (dp[x] - dp[y]));
  } else if ((dp[y] > dp[x]))
  {
    y = anc(y, (dp[y] - dp[x]));
  }
  if ((x == y))
  {
    return x;
  }
  {
    var i = (31 - builtin_clz(dp[x]));
    while ((i >= 0))
    {
      if ((fa[x][i] != fa[y][i]))
      {
        x = fa[x][i];
        y = fa[y][i];
      }
      i -= 1;
    }
  }
  return fa[x][0];
}

var rnk = cpp_array(22, (300300 << 1));

var ord = cpp_array((300300 << 1));

var dro = cpp_array((300300 << 1));

var cnt = cpp_array((300300 << 1));

var prf = cpp_array((300300 << 1));

var suf = cpp_array((300300 << 1));

func build(s: dynamic)
{
  var m = (2 * n);
  var p = 256;
  {
    var i = 1;
    while ((i <= n))
    {
      rnk[i][0] = cpp_assign(rnk[(i + n)][0], "=", s[i]);
      i += 1;
    }
  }
  {
    var j = 1;
    while ((j <= (31 - builtin_clz(n))))
    {
      {
        var i = 1;
        while ((i <= n))
        {
          var pa = fa[i][(j - 1)];
          prf[i] = rnk[i][(j - 1)];
          suf[i] = rnk[pa][(j - 1)];
          prf[(i + n)] = rnk[if (pa) (pa + n) else pa][(j - 1)];
          suf[(i + n)] = rnk[(i + n)][(j - 1)];
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i <= p))
        {
          cnt[i] = 0;
          i += 1;
        }
      }
      {
        var i = 1;
        while ((i <= m))
        {
          cnt[suf[i]] += 1;
          i += 1;
        }
      }
      {
        var i = 1;
        while ((i <= p))
        {
          cnt[i] += cnt[(i - 1)];
          i += 1;
        }
      }
      {
        var i = 1;
        while ((i <= m))
        {
          dro[cpp_update(cnt[suf[i]], "--")] = i;
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i <= p))
        {
          cnt[i] = 0;
          i += 1;
        }
      }
      {
        var i = 1;
        while ((i <= m))
        {
          cnt[prf[i]] += 1;
          i += 1;
        }
      }
      {
        var i = 1;
        while ((i <= p))
        {
          cnt[i] += cnt[(i - 1)];
          i += 1;
        }
      }
      {
        var i = m;
        while ((i >= 1))
        {
          ord[cpp_update(cnt[prf[dro[i]]], "--")] = dro[i];
          i -= 1;
        }
      }
      p = 0;
      {
        var i = 1;
        while ((i <= m))
        {
          if (((prf[ord[i]] != prf[ord[(i - 1)]]) || (suf[ord[i]] != suf[ord[(i - 1)]])))
          {
            p += 1;
          }
          rnk[ord[i]][j] = p;
          i += 1;
        }
      }
      j += 1;
    }
  }
}

func lcp(x: dynamic, y: dynamic, d: dynamic)
{
  if ((!d))
  {
    return 0;
  }
  var rlt = 0;
  {
    var i = (31 - builtin_clz(d));
    while ((i >= 0))
    {
      if (((rlt + ((1 << i))) > d))
      {
        i -= 1;
        continue;
      }
      var xx = if ((x <= n)) x else (anc((x - n), ((d - rlt) - ((1 << i)))) + n);
      var yy = if ((y <= n)) y else (anc((y - n), ((d - rlt) - ((1 << i)))) + n);
      if ((rnk[xx][i] == rnk[yy][i]))
      {
        rlt += (1 << i);
        if ((x <= n))
        {
          x = fa[x][i];
        }
        if ((y <= n))
        {
          y = fa[y][i];
        }
      }
      i -= 1;
    }
  }
  return rlt;
}

func query(a: dynamic, b: dynamic, c: dynamic, d: dynamic)
{
  var ab = lca(a, b);
  var cd = lca(c, d);
  if (((dp[a] - dp[ab]) < (dp[c] - dp[cd])))
  {
    a ^= cpp_assign(c, "^=", cpp_assign(a, "^=", c));
    b ^= cpp_assign(d, "^=", cpp_assign(b, "^=", d));
    ab ^= cpp_assign(cd, "^=", cpp_assign(ab, "^=", cd));
  }
  var del = (dp[c] - dp[cd]);
  var rlt = lcp(a, c, del);
  if ((rlt < del))
  {
    return rlt;
  }
  a = anc(a, del);
  var d1 = ((dp[a] - dp[ab]) + 1);
  var d2 = ((dp[d] - dp[cd]) + 1);
  if ((d1 >= d2))
  {
    return (rlt + lcp(a, (d + n), d2));
  }
  var md = anc(d, (d2 - d1));
  var add = lcp(a, (md + n), d1);
  rlt += add;
  if ((add < d1))
  {
    return rlt;
  }
  d1 = (dp[b] - dp[ab]);
  d2 = (dp[d] - dp[md]);
  if ((d1 > d2))
  {
    b = anc(b, (d1 - d2));
  } else
  {
    d = anc(d, (d2 - d1));
  }
  return (rlt + lcp((b + n), (d + n), min(d1, d2)));
}

var s = cpp_array(300300);

func main()
{
  scanf("%d", (&n));
  scanf("%s", (s + 1));
  {
    var i = 1;
    while ((i < n))
    {
      var x: dynamic;
      var y: dynamic;
      scanf("%d %d", (&x), (&y));
      add_edge(x, y, i);
      add_edge(y, x, (i + n));
      i += 1;
    }
  }
  dfs(1);
  build(s);
  var q: dynamic;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  scanf("%d", (&q));
  while (cpp_update(q, "--"))
  {
    scanf("%d %d %d %d", (&a), (&b), (&c), (&d));
    printf("%d\n", query(a, b, c, d));
  }
  return 0;
}
