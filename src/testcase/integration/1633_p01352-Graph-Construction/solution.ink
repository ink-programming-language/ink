// Translated from solution.cpp.

class UnionFind
{
  var size: dynamic = cpp_uninitialized();
  var parent: dynamic = cpp_array(40000);
  var rank: dynamic = cpp_array(40000);
  func UnionFind() -> dynamic
  {
    }
  func UnionFind(sz: dynamic) -> dynamic
  {
      init(sz);
    }
  func init(sz: dynamic) -> dynamic
  {
      size = sz;
      fill(rank, (rank + sz), 0);
      {
        var i: dynamic = 0;
        while ((i < sz))
        {
          parent[i] = i;
          i += 1;
        }
      }
    }
  func root(n: dynamic) -> dynamic
  {
      if ((parent[n] == n))
      {
        return n;
      }
      return cpp_assign(parent[n], "=", root(parent[n]));
    }
  func unite(n: dynamic, m: dynamic) -> dynamic
  {
      var x: dynamic = root(n);
      var y: dynamic = root(m);
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
  func isUnited(n: dynamic, m: dynamic) -> dynamic
  {
      return (root(n) == root(m));
    }
}

class Query
{
  var type_cpp: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
}

var bucket: dynamic = 100;

func make_table(use: dynamic) -> dynamic
{
  var res: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  for (var p: dynamic in use)
  {
    s.insert(p.first);
    s.insert(p.second);
  }
  var n: dynamic = 0;
  for (var v: dynamic in s)
  {
    res[v] = cpp_update(n, "++");
  }
  return res;
}

func trim(G: dynamic, use: dynamic, tbl: dynamic) -> dynamic
{
  tbl = make_table(use);
  var i: dynamic = 0;
  var res: dynamic = cpp_construct(tbl.size());
  var uf: dynamic = cpp_construct(G.size());
  {
    var i: dynamic = 0;
    while ((i < G.size()))
    {
      for (var v: dynamic in G[i])
      {
        if ((!use.count(P(i, v))))
        {
          uf.unite(i, v);
        }
      }
      i += 1;
    }
  }
  for (var p1: dynamic in tbl)
  {
    for (var p2: dynamic in tbl)
    {
      var a: dynamic = p1.first;
      var b: dynamic = p2.first;
      if (uf.isUnited(a, b))
      {
        tbl[a] = tbl[b];
      }
    }
  }
  for (var p1: dynamic in tbl)
  {
    for (var p2: dynamic in tbl)
    {
      var a: dynamic = p1.first;
      var b: dynamic = p2.first;
      if (G[b].count(a))
      {
        res[tbl[b]].insert(tbl[a]);
      }
    }
  }
  return res;
}

func unite(G: dynamic, u: dynamic, v: dynamic) -> dynamic
{
  G[u].insert(v);
  G[v].insert(u);
}

func remove(G: dynamic, u: dynamic, v: dynamic) -> dynamic
{
  G[u].erase(v);
  G[v].erase(u);
}

func judge(G: dynamic, v: dynamic, t: dynamic, reached: dynamic) -> dynamic
{
  if ((v == t))
  {
    return true;
  }
  reached[v] = true;
  for (var to: dynamic in G[v])
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

func judge(G: dynamic, s: dynamic, t: dynamic) -> dynamic
{
  var reached: dynamic = cpp_construct(G.size(), false);
  return judge(G, s, t, reached);
}

var Use: dynamic = cpp_array(((40000 / bucket) + 1));

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  read(N, M);
  var Q: dynamic = cpp_array(40000);
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      var t: dynamic = cpp_uninitialized();
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%d%d%d", (&t), (&u), (&v));
      Q[i] = [t, u, v];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      var type_cpp: dynamic = Q[i].type_cpp;
      var u: dynamic = Q[i].u;
      var v: dynamic = Q[i].v;
      Use[(i / bucket)].insert(P(u, v));
      Use[(i / bucket)].insert(P(v, u));
      i += 1;
    }
  }
  var g: dynamic = cpp_uninitialized();
  var tbl: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      var type_cpp: dynamic = Q[i].type_cpp;
      var u: dynamic = Q[i].u;
      var v: dynamic = Q[i].v;
      if (((i % bucket) == 0))
      {
        g = trim(G, Use[(i / bucket)], tbl);
      }
      var __cpp_switch_1: dynamic = type_cpp;
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
