// Translated from solution.cpp.

var N: dynamic = (300000 + 7);

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var val: dynamic = cpp_array(N);

class edge
{
  var to: dynamic = cpp_uninitialized();
  var nex: dynamic = cpp_uninitialized();
  var wei: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array((N << 1));

var fir: dynamic = cpp_array(N);

var eid: dynamic = cpp_uninitialized();

var siz: dynamic = cpp_array(N);

var dep: dynamic = cpp_array(N);

var fa: dynamic = cpp_array(N);

var son: dynamic = cpp_array(N);

var ltp: dynamic = cpp_array(N);

var faw: dynamic = cpp_array(N);

var dfn: dynamic = cpp_array(N);

var inx: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(N);

var g: dynamic = cpp_array(N);

var bit: dynamic = cpp_array(N);

func main() -> dynamic
{
  scanf("%d%d", (&n), (&q));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lld", (val + i));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    var u: dynamic = cpp_uninitialized();
    var v: dynamic = cpp_uninitialized();
    var w: dynamic = cpp_uninitialized();
    while ((i < n))
    {
      scanf("%d%d%d", (&u), (&v), (&w));
      addedge(u, v, w);
      addedge(v, u, w);
      i += 1;
    }
  }
  dfs1(1);
  ltp[1] = 1;
  dfs2(1);
  dfs3(1, 0);
  build();
  {
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    var z: dynamic = cpp_uninitialized();
    while (q)
    {
      scanf("%d%d", (&x), (&y));
      if ((x == y))
      {
        printf("%lld\n", g[x]);
      } else
      {
        z = lca(x, y);
        printf("%lld\n", ((g[z] + get(x, z)) + get(y, z)));
      }
      q -= 1;
    }
  }
  return 0;
}

func addedge(u: dynamic, v: dynamic, w: dynamic) -> dynamic
{
  e[cpp_update(eid, "++")] = [v, fir[u], (1 * w)];
  fir[u] = eid;
}

func dfs1(s: dynamic) -> dynamic
{
  siz[s] = 1;
  f[s] = val[s];
  {
    var i: dynamic = fir[s];
    while (i)
    {
      if ((e[i].to == fa[s]))
      {
        i = e[i].nex;
        continue;
      }
      fa[e[i].to] = s;
      faw[e[i].to] = e[i].wei;
      dep[e[i].to] = (dep[s] + 1);
      dfs1(e[i].to);
      siz[s] += siz[e[i].to];
      son[s] =  ((siz[e[i].to] > siz[son[s]])) ? e[i].to : son[s];
      f[s] += max(0, (f[e[i].to] - (2 * e[i].wei)));
      i = e[i].nex;
    }
  }
}

func dfs2(s: dynamic) -> dynamic
{
  dfn[s] = cpp_update(inx, "++");
  if ((!son[s]))
  {
    return;
  }
  ltp[son[s]] = ltp[s];
  dfs2(son[s]);
  {
    var i: dynamic = fir[s];
    while (i)
    {
      if (((e[i].to != fa[s]) && (e[i].to != son[s])))
      {
        ltp[e[i].to] = e[i].to;
        dfs2(e[i].to);
      }
      i = e[i].nex;
    }
  }
}

func dfs3(s: dynamic, up: dynamic) -> dynamic
{
  g[s] = (f[s] + max(0, up));
  {
    var i: dynamic = fir[s];
    while (i)
    {
      if ((e[i].to != fa[s]))
      {
        dfs3(e[i].to, ((g[s] - max(0, (f[e[i].to] - (2 * e[i].wei)))) - (2 * e[i].wei)));
      }
      i = e[i].nex;
    }
  }
}

func build() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      add(dfn[i], ((f[i] - max(0, (f[i] - (2 * faw[i])))) - faw[i]));
      i += 1;
    }
  }
}

func lca(x: dynamic, y: dynamic) -> dynamic
{
  {
    while ((ltp[x] != ltp[y]))
    {
      if ((dep[ltp[x]] < dep[ltp[y]]))
      {
        swap(x, y);
      }
      x = fa[ltp[x]];
    }
  }
  return  ((dep[x] < dep[y])) ? x : y;
}

func get(x: dynamic, z: dynamic) -> dynamic
{
  var t: dynamic = 0;
  {
    while ((ltp[x] != ltp[z]))
    {
      t += (sum(dfn[x]) - sum((dfn[ltp[x]] - 1)));
      x = fa[ltp[x]];
    }
  }
  return ((t + sum(dfn[x])) - sum(dfn[z]));
}

func add(i: dynamic, v: dynamic) -> dynamic
{
  {
    while ((i <= n))
    {
      bit[i] += v;
      i += (i & (-i));
    }
  }
}

func sum(i: dynamic) -> dynamic
{
  var t: dynamic = 0;
  {
    while (i)
    {
      t += bit[i];
      i -= (i & (-i));
    }
  }
  return t;
}
