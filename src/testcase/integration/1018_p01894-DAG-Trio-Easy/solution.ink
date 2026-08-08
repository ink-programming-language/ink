// Translated from solution.cpp.

var int_cpp = dynamic;

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

func reps(i: dynamic, f: dynamic, n: dynamic)
{
  cpp_macro("for(int i=(f);i<(n);i++)");
}

func all(v: dynamic)
{
  return cpp_expression("#include<bits/stdc++.");
}

func each(it: dynamic, v: dynamic)
{
  cpp_macro("for(__typeof((v).begin()) it=(v).begin();it!=(v).end();it++)");
}

var pb = cpp_expression("#include<");

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
  }
}

func visit(G: dynamic, vs: dynamic, used: dynamic, v: dynamic)
{
  used[v] = true;
  for (var u in G[v])
  {
    if ((!used[u]))
    {
      visit(G, vs, used, u);
    }
  }
  vs.push_back(v);
}

func visit2(T: dynamic, used: dynamic, comp: dynamic, vec: dynamic, k: dynamic, v: dynamic)
{
  comp[v] = k;
  used[v] = true;
  vec.push_back(v);
  for (var u in T[v])
  {
    if ((!used[u]))
    {
      visit2(T, used, comp, vec, k, u);
    }
  }
}

func decompose(G: dynamic, H: dynamic, comp: dynamic)
{
  var T = cpp_construct(G.size());
  {
    var i = 0;
    while ((i < G.size()))
    {
      for (var v in G[i])
      {
        T[v].push_back(i);
      }
      i += 1;
    }
  }
  comp.resize(G.size());
  var vs = cpp_construct(G.size());
  var used = cpp_construct(G.size());
  {
    var i = 0;
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
  var K = 0;
  var S: dynamic;
  for (var v in vs)
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
    var i = 0;
    while ((i < K))
    {
      for (var v in S[i])
      {
        for (var u in G[v])
        {
          if ((used[comp[u]] || (comp[v] == comp[u])))
          {
            continue;
          }
          used[comp[u]] = true;
          H[comp[v]].push_back(comp[u]);
        }
      }
      for (var v in H[i])
      {
        used[v] = false;
      }
      i += 1;
    }
  }
}

class UF
{
  var par: dynamic;
  var sz: dynamic;
  func init(n: dynamic)
  {
      par.resize(n);
      sz.resize(n);
      {
        var i = 0;
        while ((i < n))
        {
          par[i] = i;
          sz[i] = 1;
          i += 1;
        }
      }
    }
  func find(x: dynamic)
  {
      return if ((x == par[x])) x else cpp_assign(par[x], "=", find(par[x]));
    }
  func unite(x: dynamic, y: dynamic)
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
  func same(x: dynamic, y: dynamic)
  {
      return (find(x) == find(y));
    }
  func size(x: dynamic)
  {
      return sz[find(x)];
    }
}

var G: dynamic;

var bridge: dynamic;

var ord = cpp_array(1000);

var low = cpp_array(1000);

var vis = cpp_array(1000);

func dfs(v: dynamic, p: dynamic, k: dynamic)
{
  vis[v] = true;
  ord[v] = cpp_update(k, "++");
  low[v] = ord[v];
  {
    var i = 0;
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

func main()
{
  var N: dynamic;
  var M: dynamic;
  var A = cpp_array(1000);
  var B = cpp_array(1000);
  read(N, M);
  rep(i, M);
  read(A[i], B[i]);
  A[i] -= 1;
  B[i] -= 1;
  write("NO", "\n");
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var uf: dynamic;
    uf.init(N);
    rep(j, M);
    if ((i != j))
    {
      uf.unite(A[j], B[j]);
    }
    var ok = true;
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
    var H: dynamic;
    var comp: dynamic;
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
    var K = 0;
    bridge.clear();
    dfs(0, -1, K);
    if ((bridge.size() >= 2))
    {
      write("YES", "\n");
      return 0;
    }
  }
