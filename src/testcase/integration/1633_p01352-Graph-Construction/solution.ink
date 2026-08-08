// Translated from solution.cpp.

class UnionFind
{
  var size: dynamic;
  var parent: dynamic = cpp_array(40000);
  var rank: dynamic = cpp_array(40000);
  func UnionFind()
  {
    }
  func UnionFind(sz: dynamic)
  {
      init(sz);
    }
  func init(sz: dynamic)
  {
      size = sz;
      fill(rank, (rank + sz), 0);
      {
        var i = 0;
        while ((i < sz))
        {
          parent[i] = i;
          i += 1;
        }
      }
    }
  func root(n: dynamic)
  {
      if ((parent[n] == n))
      {
        return n;
      }
      return cpp_assign(parent[n], "=", root(parent[n]));
    }
  func unite(n: dynamic, m: dynamic)
  {
      var x = root(n);
      var y = root(m);
      if ((x == y))
      {
        return;
      }
      if ((rank[x] < rank[y]))
      {
        parent[x] = y;
      } else if ((rank[x] > rank[y]))
      {
        parent[y] = x;
      } else
      {
        parent[x] = y;
        rank[y] += 1;
      }
    }
  func isUnited(n: dynamic, m: dynamic)
  {
      return (root(n) == root(m));
    }
}

class Query
{
  var type_cpp: dynamic;
  var u: dynamic;
  var v: dynamic;
}

var bucket = 100;

func make_table(use: dynamic)
{
  var res: dynamic;
  var s: dynamic;
  for (var p in use)
  {
    s.insert(p.first);
    s.insert(p.second);
  }
  var n = 0;
  for (var v in s)
  {
    res[v] = cpp_update(n, "++");
  }
  return res;
}

func trim(G: dynamic, use: dynamic, tbl: dynamic)
{
  tbl = make_table(use);
  var i = 0;
  var res = cpp_construct(tbl.size());
  var uf = cpp_construct(G.size());
  {
    var i = 0;
    while ((i < G.size()))
    {
      for (var v in G[i])
      {
        if ((!use.count(P(i, v))))
        {
          uf.unite(i, v);
        }
      }
      i += 1;
    }
  }
  for (var p1 in tbl)
  {
    for (var p2 in tbl)
    {
      var a = p1.first;
      var b = p2.first;
      if (uf.isUnited(a, b))
      {
        tbl[a] = tbl[b];
      }
    }
  }
  for (var p1 in tbl)
  {
    for (var p2 in tbl)
    {
      var a = p1.first;
      var b = p2.first;
      if (G[b].count(a))
      {
        res[tbl[b]].insert(tbl[a]);
      }
    }
  }
  return res;
}

func unite(G: dynamic, u: dynamic, v: dynamic)
{
  G[u].insert(v);
  G[v].insert(u);
}

func remove(G: dynamic, u: dynamic, v: dynamic)
{
  G[u].erase(v);
  G[v].erase(u);
}

func judge(G: dynamic, v: dynamic, t: dynamic, reached: dynamic)
{
  if ((v == t))
  {
    return true;
  }
  reached[v] = true;
  for (var to in G[v])
  {
    if ((!reached[to]))
    {
      if (judge(G, to, t, reached))
      {
        return true;
      }
    }
  }
  return false;
}

func judge(G: dynamic, s: dynamic, t: dynamic)
{
  var reached = cpp_construct(G.size(), false);
  return judge(G, s, t, reached);
}

var Use = cpp_array(((40000 / bucket) + 1));

func main()
{
  var N: dynamic;
  var M: dynamic;
  read(N, M);
  var Q = cpp_array(40000);
  {
    var i = 0;
    while ((i < M))
    {
      var t: dynamic;
      var u: dynamic;
      var v: dynamic;
      scanf("%d%d%d", (&t), (&u), (&v));
      Q[i] = [t, u, v];
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < M))
    {
      var type_cpp = Q[i].type_cpp;
      var u = Q[i].u;
      var v = Q[i].v;
      Use[(i / bucket)].insert(P(u, v));
      Use[(i / bucket)].insert(P(v, u));
      i += 1;
    }
  }
  var g: dynamic;
  var tbl: dynamic;
  {
    var i = 0;
    while ((i < M))
    {
      var type_cpp = Q[i].type_cpp;
      var u = Q[i].u;
      var v = Q[i].v;
      if (((i % bucket) == 0))
      {
        g = trim(G, Use[(i / bucket)], tbl);
      }
      var __cpp_switch_1 = type_cpp;
      if (__cpp_switch_1 == 1)
      {
        unite(G, u, v);
        unite(g, tbl[u], tbl[v]);
        break;
      }
      else if (__cpp_switch_1 == 2)
      {
        remove(G, u, v);
        remove(g, tbl[u], tbl[v]);
        break;
      }
      else if (__cpp_switch_1 == 3)
      {
        if (judge(g, tbl[u], tbl[v]))
        {
        write("YES", "\n");
        } else
        {
        write("NO", "\n");
        }
        break;
      }
      i += 1;
    }
  }
}
