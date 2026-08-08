// Translated from solution.cpp.

var visual = cpp_expression("usin");

var g: dynamic;

var h: dynamic;

var seen: dynamic;

var seen2: dynamic;

var dist: dynamic;

var gcds: dynamic;

var par: dynamic;

var comp: dynamic;

var cur = 0;

func gcd(a: dynamic, b: dynamic)
{
  if (((a == 0) || (b == 0)))
  {
    return (a + b);
  }
  if ((a < b))
  {
    return gcd(b, a);
  }
  return gcd((a % b), b);
}

func dfs(v: dynamic, p: dynamic, d: dynamic)
{
  if (seen[v])
  {
    return;
  }
  par[v] = cur;
  dist[v] = d;
  seen[v] = true;
  {
    var i = 0;
    while ((i < g[v].size()))
    {
      var u = g[v][i].first;
      if ((comp[u] != comp[v]))
      {
        i += 1;
        continue;
      }
      dfs(u, v, (d + g[v][i].second));
      i += 1;
    }
  }
}

func dfs2(v: dynamic, p: dynamic, d: dynamic)
{
  gcds[cur] = gcd(gcds[cur], (d + dist[v]));
  if (seen2[v])
  {
    return;
  }
  par[v] = cur;
  seen2[v] = true;
  {
    var i = 0;
    while ((i < h[v].size()))
    {
      var u = h[v][i].first;
      if ((comp[u] != comp[v]))
      {
        i += 1;
        continue;
      }
      dfs2(u, v, (d + h[v][i].second));
      i += 1;
    }
  }
}

var SZ = 0;

var ID: dynamic;

var LS = 0;

var L: dynamic;

var R: dynamic;

var gg: dynamic;

func dfs1(v: dynamic)
{
  if (ID[v])
  {
    return;
  }
  ID[v] = 1;
  for (var u in gg[v])
  {
    dfs1(u);
  }
  L[cpp_update(LS, "++")] = v;
}

func dfs2(v: dynamic, r: dynamic)
{
  if ((~ID[v]))
  {
    return;
  }
  ID[v] = r;
  for (var u in R[v])
  {
    dfs2(u, r);
  }
}

func scc()
{
  var n = g.size();
  R.resize(n);
  ID.resize(n);
  L.resize(n);
  {
    var i = 0;
    while ((i < n))
    {
      for (var u in gg[i])
      {
        R[u].push_back(i);
      }
      i += 1;
    }
  }
  {
    var v = 0;
    while ((v < n))
    {
      dfs1(v);
      v += 1;
    }
  }
  fill(ID.begin(), ID.end(), -1);
  var mx = 0;
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      if ((ID[L[i]] == -1))
      {
        dfs2(L[i], cpp_update(SZ, "++"));
      }
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      mx = max(mx, ID[i]);
      i += 1;
    }
  }
  var A = cpp_construct((mx + 1));
  var B = cpp_construct((mx + 1));
  {
    var u = 0;
    while ((u < n))
    {
      A[ID[u]].push_back(u);
      for (var v in gg[u])
      {
        if ((ID[u] != ID[v]))
        {
          B[ID[u]].push_back(ID[v]);
        }
      }
      u += 1;
    }
  }
  {
    var i = 0;
    while ((i < SZ))
    {
      sort(B[i].begin(), B[i].end());
      B[i].erase(unique(B[i].begin(), B[i].end()), B[i].end());
      i += 1;
    }
  }
  return [A, B];
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  var m: dynamic;
  var q: dynamic;
  read(n, m);
  g = vector(n);
  h = vector(n);
  gg = vector(n);
  {
    var i = 0;
    while ((i < m))
    {
      var x: dynamic;
      var y: dynamic;
      var l: dynamic;
      read(x, y, l);
      x -= 1;
      y -= 1;
      g[x].push_back([y, l]);
      gg[x].push_back(y);
      h[y].push_back([x, l]);
      i += 1;
    }
  }
  var aa = scc().first;
  comp = vector(n, false);
  var ii = 0;
  for (var a in aa)
  {
    for (var z in a)
    {
      comp[z] = ii;
    }
    ii += 1;
  }
  seen = vector(n, false);
  seen2 = vector(n, false);
  dist = vector(n, false);
  gcds = vector(n, false);
  par = vector(n, false);
  {
    var i = 0;
    while ((i < n))
    {
      if ((!seen[i]))
      {
        cur = i;
        dfs(i, -1, 0);
        dfs2(i, -1, 0);
      }
      i += 1;
    }
  }
  read(q);
  {
    var i = 0;
    while ((i < q))
    {
      var v: dynamic;
      var s: dynamic;
      var t: dynamic;
      read(v, s, t);
      v -= 1;
      if (((s == 0) || ((s % gcd(gcds[par[v]], t)) == 0)))
      {
        write("YES", "\n");
      } else
      {
        write("NO", "\n");
      }
      i += 1;
    }
  }
}
