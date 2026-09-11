// Translated from solution.cpp.

var visual: dynamic = cpp_expression("usin");

var g: dynamic = cpp_uninitialized();

var h: dynamic = cpp_uninitialized();

var seen: dynamic = cpp_uninitialized();

var seen2: dynamic = cpp_uninitialized();

var dist: dynamic = cpp_uninitialized();

var gcds: dynamic = cpp_uninitialized();

var par: dynamic = cpp_uninitialized();

var comp: dynamic = cpp_uninitialized();

var cur: dynamic = 0;

func gcd(a: dynamic, b: dynamic) -> dynamic
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

func dfs(v: dynamic, p: dynamic, d: dynamic) -> dynamic
{
  if (seen[v])
  {
    return;
  }
  par[v] = cur;
  dist[v] = d;
  seen[v] = true;
  {
    var i: dynamic = 0;
    while ((i < g[v].size()))
    {
      var u: dynamic = g[v][i].first;
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

func dfs2(v: dynamic, p: dynamic, d: dynamic) -> dynamic
{
  gcds[cur] = gcd(gcds[cur], (d + dist[v]));
  if (seen2[v])
  {
    return;
  }
  par[v] = cur;
  seen2[v] = true;
  {
    var i: dynamic = 0;
    while ((i < h[v].size()))
    {
      var u: dynamic = h[v][i].first;
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

var SZ: dynamic = 0;

var ID: dynamic = cpp_uninitialized();

var LS: dynamic = 0;

var L: dynamic = cpp_uninitialized();

var R: dynamic = cpp_uninitialized();

var gg: dynamic = cpp_uninitialized();

func dfs1(v: dynamic) -> dynamic
{
  if (ID[v])
  {
    return;
  }
  ID[v] = 1;
  for (var u: dynamic in gg[v])
  {
    dfs1(u);
  }
  L[cpp_update(LS, "++")] = v;
}

func dfs2(v: dynamic, r: dynamic) -> dynamic
{
  if ((~ID[v]))
  {
    return;
  }
  ID[v] = r;
  for (var u: dynamic in R[v])
  {
    dfs2(u, r);
  }
}

func scc() -> dynamic
{
  var n: dynamic = g.size();
  R.resize(n);
  ID.resize(n);
  L.resize(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      for (var u: dynamic in gg[i])
      {
        R[u].push_back(i);
      }
      i += 1;
    }
  }
  {
    var v: dynamic = 0;
    while ((v < n))
    {
      dfs1(v);
      v += 1;
    }
  }
  fill(ID.begin(), ID.end(), -1);
  var mx: dynamic = 0;
  {
    var i: dynamic = (n - 1);
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
    var i: dynamic = 0;
    while ((i < n))
    {
      mx = max(mx, ID[i]);
      i += 1;
    }
  }
  var A: dynamic = cpp_construct((mx + 1));
  var B: dynamic = cpp_construct((mx + 1));
  {
    var u: dynamic = 0;
    while ((u < n))
    {
      A[ID[u]].push_back(u);
      for (var v: dynamic in gg[u])
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
    var i: dynamic = 0;
    while ((i < SZ))
    {
      sort(B[i].begin(), B[i].end());
      B[i].erase(unique(B[i].begin(), B[i].end()), B[i].end());
      i += 1;
    }
  }
  return [A, B];
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(n, m);
  g = vector(n);
  h = vector(n);
  gg = vector(n);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      var l: dynamic = cpp_uninitialized();
      read(x, y, l);
      x -= 1;
      y -= 1;
      g[x].push_back([y, l]);
      gg[x].push_back(y);
      h[y].push_back([x, l]);
      i += 1;
    }
  }
  var aa: dynamic = scc().first;
  comp = vector(n, false);
  var ii: dynamic = 0;
  for (var a: dynamic in aa)
  {
    for (var z: dynamic in a)
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
    var i: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < q))
    {
      var v: dynamic = cpp_uninitialized();
      var s: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
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
