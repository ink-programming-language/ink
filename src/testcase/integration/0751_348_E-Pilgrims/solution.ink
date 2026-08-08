// Translated from solution.cpp.

func read()
{
  var ch = getchar();
  var x = 0;
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return x;
}

var maxn = (2e5 + 5);

var inf = 1e9;

class Data
{
  var next: dynamic;
  var to: dynamic;
  var w: dynamic;
}

class LinkTable
{
  var data: dynamic = cpp_array(maxn);
  var head: dynamic = cpp_array(maxn);
  var cnt: dynamic;
  func add(x: dynamic, y: dynamic, w: dynamic)
  {
      data[cpp_update(cnt, "++")] = [head[x], y, w];
      head[x] = cnt;
    }
}

var E: dynamic;

var R: dynamic;

class DP
{
  var sum: dynamic;
  var id: dynamic;
  var lca: dynamic;
}

var f = cpp_array(maxn);

var g = cpp_array(maxn);

var N: dynamic;

var K: dynamic;

var mx = cpp_array(5);

var mi = cpp_array(5);

var ret = cpp_array(maxn);

var fa = cpp_array(maxn);

var w = cpp_array(maxn);

var mark = cpp_array(maxn);

func update(x: dynamic, y: dynamic)
{
  if ((x < y))
  {
    return cpp_comma(cpp_assign(x, "=", y), 1);
  }
  if ((x == y))
  {
    return 0;
  }
  return -1;
}

func F(x: dynamic)
{
  var sum = if (mark[x]) 0 else (-inf);
  var id = x;
  {
    var i = E.head[x];
    while (i)
    {
      if ((E.data[i].to != fa[x]))
      {
        var e = E.data[i];
        fa[e.to] = x;
        w[e.to] = e.w;
        F(e.to);
        if ((update(sum, (f[e.to].sum + e.w)) == 1))
        {
          id = f[e.to].id;
        } else if ((!update(sum, (f[e.to].sum + e.w))))
        {
          id = x;
        }
      }
      i = E.data[i].next;
    }
  }
  f[x].sum = sum;
  f[x].id = id;
  f[x].lca = x;
}

func G(x: dynamic)
{
  g[x] = g[fa[x]];
  g[x].sum += w[x];
  if ((mark[fa[x]] && (w[x] > g[x].sum)))
  {
    g[x].sum = w[x];
    g[x].id = cpp_assign(g[x].lca, "=", fa[x]);
  }
  {
    var i = R.head[fa[x]];
    while (i)
    {
      if ((R.data[i].to != x))
      {
        var e = R.data[i];
        if ((update(g[x].sum, ((f[e.to].sum + e.w) + w[x])) == 1))
        {
          g[x].id = f[e.to].id;
          g[x].lca = fa[x];
        } else if ((update(g[x].sum, ((f[e.to].sum + e.w) + w[x])) == 0))
        {
          g[x].id = cpp_assign(g[x].lca, "=", fa[x]);
        }
      }
      i = R.data[i].next;
    }
  }
  {
    var i = 0;
    while ((i < 3))
    {
      mi[i] = 0;
      mx[i] = (-inf);
      i += 1;
    }
  }
  {
    var i = E.head[x];
    while (i)
    {
      if ((E.data[i].to != fa[x]))
      {
        var e = E.data[i];
        if ((update(mx[0], (f[e.to].sum + e.w)) == 1))
        {
          mx[2] = mx[1];
          mi[2] = mi[1];
          mx[1] = mx[0];
          mi[1] = mi[0];
          mi[0] = e.to;
        } else if ((update(mx[1], (f[e.to].sum + e.w)) == 1))
        {
          mx[2] = mx[1];
          mi[2] = mi[1];
          mi[1] = e.to;
        } else if ((update(mx[2], (f[e.to].sum + e.w)) == 1))
        {
          mi[2] = e.to;
        }
      }
      i = E.data[i].next;
    }
  }
  {
    var i = 0;
    while ((i < 3))
    {
      if (mi[i])
      {
        R.add(x, mi[i], w[mi[i]]);
      }
      i += 1;
    }
  }
  {
    var i = E.head[x];
    while (i)
    {
      if ((E.data[i].to != fa[x]))
      {
        G(E.data[i].to);
      }
      i = E.data[i].next;
    }
  }
}

func init()
{
  N = read();
  K = read();
  {
    var i = 1;
    while ((i <= K))
    {
      mark[read()] = true;
      i += 1;
    }
  }
  {
    var i = 1;
    var u: dynamic;
    var v: dynamic;
    var w: dynamic;
    while ((i < N))
    {
      u = read();
      v = read();
      w = read();
      E.add(u, v, w);
      E.add(v, u, w);
      i += 1;
    }
  }
  g[0].sum = (-inf);
}

func dfs(x: dynamic)
{
  {
    var i = E.head[x];
    while (i)
    {
      if ((E.data[i].to != fa[x]))
      {
        var e = E.data[i];
        dfs(e.to);
        ret[x] += ret[e.to];
      }
      i = E.data[i].next;
    }
  }
}

func solve()
{
  {
    var i = 1;
    while ((i <= N))
    {
      if ((mark[i] && (f[i].sum != g[i].sum)))
      {
        if ((f[i].sum > g[i].sum))
        {
          ret[f[i].id] += 1;
          ret[f[i].lca] -= 1;
        } else
        {
          ret[g[i].id] += 1;
          ret[g[i].lca] -= 1;
          ret[i] += 1;
          ret[fa[g[i].lca]] -= 1;
        }
      }
      i += 1;
    }
  }
  dfs(1);
  var sum = (-inf);
  var once = 0;
  {
    var i = 1;
    while ((i <= N))
    {
      if ((!mark[i]))
      {
        if ((ret[i] > sum))
        {
          sum = ret[i];
          once = 1;
        } else if ((ret[i] == sum))
        {
          once += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d %d\n", sum, once);
}

func main()
{
  init();
  F(1);
  G(1);
  solve();
  return 0;
}
