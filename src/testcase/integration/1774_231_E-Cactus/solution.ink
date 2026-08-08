// Translated from solution.cpp.

func read()
{
  var x = 0;
  var f = 1;
  var c = getchar();
  while (((c < cpp_char("0")) || (c > cpp_char("9"))))
  {
    if ((c == cpp_char("-")))
    {
      f = 0;
    }
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = ((((x << 3)) + ((x << 1))) + ((c ^ 48)));
    c = getchar();
  }
  return if (f) x else (-x);
}

var N = (1e5 + 5);

var M = (2e5 + 5);

var mod = (1e9 + 7);

var tot = 1;

var ver = cpp_array(M);

var nxt = cpp_array(M);

var head = cpp_array(N);

func add(u: dynamic, v: dynamic)
{
  ver[cpp_update(tot, "++")] = v;
  nxt[tot] = head[u];
  head[u] = tot;
}

var tot2: dynamic;

var ver2 = cpp_array(M);

var nxt2 = cpp_array(M);

var head2 = cpp_array(N);

func Add(u: dynamic, v: dynamic)
{
  ver2[cpp_update(tot2, "++")] = v;
  nxt2[tot2] = head2[u];
  head2[u] = tot2;
}

var dfn = cpp_array(N);

var low = cpp_array(N);

var num: dynamic;

var bridge = cpp_array(M);

func tarjan(u: dynamic, lst: dynamic)
{
  dfn[u] = cpp_assign(low[u], "=", cpp_update(num, "++"));
  {
    var i = head[u];
    while (i)
    {
      var v = ver[i];
      if ((!dfn[v]))
      {
        tarjan(v, i);
        low[u] = min(low[u], low[v]);
        if ((low[v] > dfn[u]))
        {
          bridge[i] = cpp_assign(bridge[(i ^ 1)], "=", 1);
        }
      } else if ((i != ((lst ^ 1))))
      {
        low[u] = min(low[u], dfn[v]);
      }
      i = nxt[i];
    }
  }
}

var col = cpp_array(N);

var cnt = cpp_array(N);

var dcc: dynamic;

func dfs1(u: dynamic)
{
  col[u] = dcc;
  cnt[dcc] += 1;
  {
    var i = head[u];
    while (i)
    {
      var v = ver[i];
      if ((bridge[i] || col[v]))
      {
        i = nxt[i];
        continue;
      }
      dfs1(v);
      i = nxt[i];
    }
  }
}

var dis = cpp_array(N);

func dfs2(u: dynamic, fa: dynamic)
{
  dis[u] += ((cnt[u] > 1));
  {
    var i = head2[u];
    while (i)
    {
      var v = ver2[i];
      if ((v == fa))
      {
        i = nxt2[i];
        continue;
      }
      dis[v] = dis[u];
      dfs2(v, u);
      i = nxt2[i];
    }
  }
}

var dep = cpp_array(N);

var f = cpp_array(30, N);

var t = 20;

var q: dynamic;

func bfs(st: dynamic)
{
  q.push(st);
  dep[st] = 1;
  while ((!q.empty()))
  {
    var u = q.front();
    q.pop();
    {
      var i = head2[u];
      while (i)
      {
        var v = ver2[i];
        if (dep[v])
        {
          i = nxt2[i];
          continue;
        }
        dep[v] = (dep[u] + 1);
        f[v][0] = u;
        {
          var j = 1;
          while ((j <= t))
          {
            f[v][j] = f[f[v][(j - 1)]][(j - 1)];
            j += 1;
          }
        }
        q.push(v);
        i = nxt2[i];
      }
    }
  }
}

func lca(u: dynamic, v: dynamic)
{
  if ((dep[u] > dep[v]))
  {
    swap(u, v);
  }
  {
    var i = t;
    while ((i >= 0))
    {
      if ((dep[f[v][i]] >= dep[u]))
      {
        v = f[v][i];
      }
      i -= 1;
    }
  }
  if ((u == v))
  {
    return u;
  }
  {
    var i = t;
    while ((i >= 0))
    {
      if ((f[u][i] != f[v][i]))
      {
        u = f[u][i];
        v = f[v][i];
      }
      i -= 1;
    }
  }
  return f[u][0];
}

func qpow(a: dynamic, x: dynamic)
{
  var res = 1;
  while (x)
  {
    if ((x & 1))
    {
      res = (((res * a)) % mod);
    }
    a = (((a * a)) % mod);
    x >>= 1;
  }
  return res;
}

var n: dynamic;

var m: dynamic;

var T: dynamic;

func main()
{
  n = read();
  m = read();
  {
    var i = 1;
    while ((i <= m))
    {
      var u = read();
      var v = read();
      add(u, v);
      add(v, u);
      i += 1;
    }
  }
  tarjan(1, -1);
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!col[i]))
      {
        dcc += 1;
        dfs1(i);
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = head[i];
        while (j)
        {
          var v = ver[j];
          if ((col[i] != col[v]))
          {
            Add(col[i], col[v]);
          }
          j = nxt[j];
        }
      }
      i += 1;
    }
  }
  dfs2(1, -1);
  bfs(1);
  T = read();
  while (cpp_update(T, "--"))
  {
    var u = col[read()];
    var v = col[read()];
    var l = lca(u, v);
    printf("%lld\n", qpow(2, (((dis[u] + dis[v]) - (dis[l] * 2)) + ((cnt[l] > 1)))));
  }
  return 0;
}
