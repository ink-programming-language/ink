// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

func reps(i: dynamic, f: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=(f);i<(n);i++)");
}

func all(v: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

func each(it: dynamic, v: dynamic) -> dynamic
{
  cpp_macro("for(__typeof((v).begin()) it=(v).begin();it!=(v).end();it++)");
}

var pb: dynamic = cpp_expression("#include<");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
  }
}

func visit(G: dynamic, vs: dynamic, used: dynamic, v: dynamic) -> dynamic
{
  used[v] = true;
  for (var u: dynamic in G[v])
  {
    if ((!used[u]))
    {
      visit(G, vs, used, u);
    }
  }
  vs.push_back(v);
}

func visit2(T: dynamic, used: dynamic, comp: dynamic, vec: dynamic, k: dynamic, v: dynamic) -> dynamic
{
  comp[v] = k;
  used[v] = true;
  vec.push_back(v);
  for (var u: dynamic in T[v])
  {
    if ((!used[u]))
    {
      visit2(T, used, comp, vec, k, u);
    }
  }
}

func decompose(G: dynamic, H: dynamic, comp: dynamic) -> dynamic
{
  var T: dynamic = cpp_construct(G.size());
  {
    var i: dynamic = 0;
    while ((i < G.size()))
    {
      for (var v: dynamic in G[i])
      {
        T[v].push_back(i);
      }
      i += 1;
    }
  }
  comp.resize(G.size());
  var vs: dynamic = cpp_construct(G.size());
  var used: dynamic = cpp_construct(G.size());
  {
    var i: dynamic = 0;
    while ((i < G.size()))
    {
      if ((!used[i]))
      {
        visit(G, vs, used, i);
      }
      i += 1;
    }
  }
  reverse(vs.begin(), vs.end());
  fill(used.begin(), used.end(), 0);
  var K: dynamic = 0;
  var S: dynamic = cpp_uninitialized();
  for (var v: dynamic in vs)
  {
    if ((!used[v]))
    {
      S.push_back(vector());
      visit2(T, used, comp, S.back(), cpp_update(K, "++"), v);
    }
  }
  H.resize(K);
  fill(used.begin(), used.end(), 0);
  {
    var i: dynamic = 0;
    while ((i < K))
    {
      for (var v: dynamic in S[i])
      {
        for (var u: dynamic in G[v])
        {
          if ((used[comp[u]] || (comp[v] == comp[u])))
          {
            continue;
          }
          used[comp[u]] = true;
          H[comp[v]].push_back(comp[u]);
        }
      }
      for (var v: dynamic in H[i])
      {
        used[v] = false;
      }
      i += 1;
    }
  }
}

class UF
{
  var par: dynamic = cpp_uninitialized();
  var sz: dynamic = cpp_uninitialized();
  func init(n: dynamic) -> dynamic
  {
      par.resize(n);
      sz.resize(n);
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          par[i] = i;
          sz[i] = 1;
          i += 1;
        }
      }
    }
  func find(x: dynamic) -> dynamic
  {
      return  ((x == par[x])) ? x : cpp_assign(par[x], "=", find(par[x]));
    }
  func unite(x: dynamic, y: dynamic) -> dynamic
  {
      x = find(x);
      y = find(y);
      if ((x == y))
      {
        return;
      }
      sz[x] += sz[y];
      par[y] = x;
    }
  func same(x: dynamic, y: dynamic) -> dynamic
  {
      return (find(x) == find(y));
    }
  func size(x: dynamic) -> dynamic
  {
      return sz[find(x)];
    }
}

var G: dynamic = cpp_uninitialized();

var bridge: dynamic = cpp_uninitialized();

var ord: dynamic = cpp_array(1000);

var low: dynamic = cpp_array(1000);

var vis: dynamic = cpp_array(1000);

func dfs(v: dynamic, p: dynamic, k: dynamic) -> dynamic
{
  vis[v] = true;
  ord[v] = cpp_update(k, "++");
  low[v] = ord[v];
  {
    var i: dynamic = 0;
    while ((i < G[v].size()))
    {
      if ((!vis[G[v][i]]))
      {
        dfs(G[v][i], v, k);
        low[v] = min(low[v], low[G[v][i]]);
        if ((ord[v] < low[G[v][i]]))
        {
          bridge.push_back(make_pair(min(v, G[v][i]), max(v, G[v][i])));
        }
      } else if ((G[v][i] != p))
      {
        low[v] = min(low[v], ord[G[v][i]]);
      }
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  var A: dynamic = cpp_array(1000);
  var B: dynamic = cpp_array(1000);
  read(N, M);
  rep(i, M);
  read(A[i], B[i]);
  A[i] -= 1;
  B[i] -= 1;
  write("NO", "\n");
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var uf: dynamic = cpp_uninitialized();
    uf.init(N);
    rep(j, M);
    if ((i != j))
    {
      uf.unite(A[j], B[j]);
    }
    var ok: dynamic = true;
    rep(j, N);
    if ((uf.find(j) != uf.find(0)))
    {
      ok = false;
    }
    if ((!ok))
    {
      continue;
    }
    G = vector(N);
    rep(j, M);
    if ((i != j))
    {
      G[A[j]].pb(B[j]);
    }
    var H: dynamic = cpp_uninitialized();
    var comp: dynamic = cpp_uninitialized();
    SCC.decompose(G, H, comp);
    if ((H.size() != N))
    {
      continue;
    }
    G = vector(N);
    rep(j, M);
    if ((i != j))
    {
      G[A[j]].pb(B[j]);
      G[B[j]].pb(A[j]);
    }
    memset(vis, 0, cpp_sizeof((vis)));
    var K: dynamic = 0;
    bridge.clear();
    dfs(0, -1, K);
    if ((bridge.size() >= 2))
    {
      write("YES", "\n");
      return 0;
    }
  }
