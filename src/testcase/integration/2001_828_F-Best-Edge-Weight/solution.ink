// Translated from solution.cpp.

var MAX_N = (2e5 + 10);

class Edge
{
  var u: dynamic;
  var v: dynamic;
  var cost: dynamic;
  var idx: dynamic;
  func operator_less(rhs: dynamic)
  {
      return (cost < rhs.cost);
    }
}

var ed = cpp_array(MAX_N);

func cmp_idx(lhs: dynamic, rhs: dynamic)
{
  return (lhs.idx < rhs.idx);
}

var ID: dynamic;

var ver = cpp_array(19, MAX_N);

var maxCost = cpp_array(19, MAX_N);

var dep = cpp_array(MAX_N);

var par = cpp_array(MAX_N);

var rank = cpp_array(MAX_N);

var n: dynamic;

var m: dynamic;

var ans = cpp_array(MAX_N);

var inMST = cpp_array(MAX_N);

var g = cpp_array(MAX_N);

func init(n: dynamic)
{
  {
    var i = 1;
    while ((i <= n))
    {
      par[i] = i;
      rank[i] = 0;
      i += 1;
    }
  }
}

func find(x: dynamic)
{
  if ((par[x] == x))
  {
    return x;
  }
  return cpp_assign(par[x], "=", find(par[x]));
}

func unite(x: dynamic, y: dynamic)
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

func same(x: dynamic, y: dynamic)
{
  return (find(x) == find(y));
}

func dfs(u: dynamic, fa: dynamic, d: dynamic)
{
  dep[u] = d;
  for (var e in g[u])
  {
    var v = e.first;
    var cost = e.second;
    if ((v != fa))
    {
      ver[v][0] = u;
      maxCost[v][0] = cost;
      dfs(v, u, (d + 1));
    }
  }
}

func initLca()
{
  memset(ver, -1, cpp_sizeof(ver));
  memset(maxCost, 0x8f, cpp_sizeof(maxCost));
  dfs(1, -1, 1);
  {
    var k = 1;
    while ((k < 19))
    {
      {
        var v = 1;
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

func find_lca(u: dynamic, v: dynamic)
{
  if ((dep[u] > dep[v]))
  {
    swap(u, v);
  }
  var res = -0x7fffffff;
  {
    var k = 18;
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
    var k = 18;
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

func modify(u: dynamic, v: dynamic, w: dynamic)
{
  v = find(v);
  while ((dep[u] < dep[v]))
  {
    var fa = ver[v][0];
    var id = ID[pair(fa, v)];
    ans[id] = min(ans[id], w);
    par[v] = find(fa);
    v = find(v);
  }
}

func main()
{
  var u: dynamic;
  var v: dynamic;
  var c: dynamic;
  scanf("%d%d", (&n), (&m));
  {
    var i = 0;
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
    var i = 0;
    while ((i < m))
    {
      var e = ed[i];
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
    var i = 0;
    while ((i < m))
    {
      if (inMST[i])
      {
        i += 1;
        continue;
      }
      var e = ed[i];
      var lca = find_lca(e.u, e.v);
      ans[e.idx] = min(ans[e.idx], (lca.first - 1));
      modify(lca.second, e.u, (e.cost - 1));
      modify(lca.second, e.v, (e.cost - 1));
      i += 1;
    }
  }
  {
    var i = 0;
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
