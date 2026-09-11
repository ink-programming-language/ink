// Translated from solution.cpp.

var MAX_N: dynamic = (2e5 + 10);

class Edge
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var cost: dynamic = cpp_uninitialized();
  var idx: dynamic = cpp_uninitialized();
  func operator_less(rhs: dynamic) -> dynamic
  {
      return (cost < rhs.cost);
    }
}

var ed: dynamic = cpp_array(MAX_N);

func cmp_idx(lhs: dynamic, rhs: dynamic) -> dynamic
{
  return (lhs.idx < rhs.idx);
}

var ID: dynamic = cpp_uninitialized();

var ver: dynamic = cpp_array(19, MAX_N);

var maxCost: dynamic = cpp_array(19, MAX_N);

var dep: dynamic = cpp_array(MAX_N);

var par: dynamic = cpp_array(MAX_N);

var rank: dynamic = cpp_array(MAX_N);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_array(MAX_N);

var inMST: dynamic = cpp_array(MAX_N);

var g: dynamic = cpp_array(MAX_N);

func init(n: dynamic) -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      par[i] = i;
      rank[i] = 0;
      i += 1;
    }
  }
}

func find(x: dynamic) -> dynamic
{
  if ((par[x] == x))
  {
    return x;
  }
  return cpp_assign(par[x], "=", find(par[x]));
}

func unite(x: dynamic, y: dynamic) -> dynamic
{
  x = find(x);
  y = find(y);
  if ((x == y))
  {
    return;
  }
  if ((rank[x] < rank[y]))
  {
    par[x] = y;
  } else
  {
    par[y] = x;
    if ((rank[x] == rank[y]))
    {
      rank[x] += 1;
    }
  }
}

func same(x: dynamic, y: dynamic) -> dynamic
{
  return (find(x) == find(y));
}

func dfs(u: dynamic, fa: dynamic, d: dynamic) -> dynamic
{
  dep[u] = d;
  for (var e: dynamic in g[u])
  {
    var v: dynamic = e.first;
    var cost: dynamic = e.second;
    if ((v != fa))
    {
      ver[v][0] = u;
      maxCost[v][0] = cost;
      dfs(v, u, (d + 1));
    }
  }
}

func initLca() -> dynamic
{
  memset(ver, -1, cpp_sizeof(ver));
  memset(maxCost, 0x8f, cpp_sizeof(maxCost));
  dfs(1, -1, 1);
  {
    var k: dynamic = 1;
    while ((k < 19))
    {
      {
        var v: dynamic = 1;
        while ((v <= n))
        {
          if (((ver[v][(k - 1)] != -1) && (ver[ver[v][(k - 1)]][(k - 1)] != -1)))
          {
            maxCost[v][k] = max(maxCost[v][(k - 1)], maxCost[ver[v][(k - 1)]][(k - 1)]);
            ver[v][k] = ver[ver[v][(k - 1)]][(k - 1)];
          }
          v += 1;
        }
      }
      k += 1;
    }
  }
}

func find_lca(u: dynamic, v: dynamic) -> dynamic
{
  if ((dep[u] > dep[v]))
  {
    swap(u, v);
  }
  var res: dynamic = -0x7fffffff;
  {
    var k: dynamic = 18;
    while ((k >= 0))
    {
      if (((ver[v][k] != -1) && (dep[ver[v][k]] >= dep[u])))
      {
        res = max(res, maxCost[v][k]);
        v = ver[v][k];
      }
      k -= 1;
    }
  }
  if ((u == v))
  {
    return pair(res, v);
  }
  {
    var k: dynamic = 18;
    while ((k >= 0))
    {
      if ((ver[v][k] != ver[u][k]))
      {
        res = max(res, max(maxCost[v][k], maxCost[u][k]));
        v = ver[v][k];
        u = ver[u][k];
      }
      k -= 1;
    }
  }
  res = max(res, max(maxCost[v][0], maxCost[u][0]));
  return pair(res, ver[v][0]);
}

func modify(u: dynamic, v: dynamic, w: dynamic) -> dynamic
{
  v = find(v);
  while ((dep[u] < dep[v]))
  {
    var fa: dynamic = ver[v][0];
    var id: dynamic = ID[pair(fa, v)];
    ans[id] = min(ans[id], w);
    par[v] = find(fa);
    v = find(v);
  }
}

func main() -> dynamic
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d%d%d", (&u), (&v), (&c));
      ed[i] = [u, v, c, i];
      ID[pair(u, v)] = i;
      ID[pair(v, u)] = i;
      ans[i] = 0x7fffffff;
      i += 1;
    }
  }
  sort(ed, (ed + m));
  init(n);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var e: dynamic = ed[i];
      if ((!same(e.u, e.v)))
      {
        unite(e.u, e.v);
        g[e.u].push_back(pair(e.v, e.cost));
        g[e.v].push_back(pair(e.u, e.cost));
        inMST[i] = true;
      }
      i += 1;
    }
  }
  initLca();
  init(n);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      if (inMST[i])
      {
        i += 1;
        continue;
      }
      var e: dynamic = ed[i];
      var lca: dynamic = find_lca(e.u, e.v);
      ans[e.idx] = min(ans[e.idx], (lca.first - 1));
      modify(lca.second, e.u, (e.cost - 1));
      modify(lca.second, e.v, (e.cost - 1));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      if ((ans[i] == 0x7fffffff))
      {
        ans[i] = -1;
      }
      printf("%d ", ans[i]);
      i += 1;
    }
  }
  puts("");
  return 0;
}
