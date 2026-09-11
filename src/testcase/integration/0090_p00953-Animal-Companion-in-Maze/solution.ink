// Translated from solution.cpp.

var INF: dynamic = (1 << 29);

class UnionFind
{
  var data: dynamic = cpp_uninitialized();
  func UnionFind(sz: dynamic) -> dynamic
  {
      data.assign(sz, -1);
    }
  func find(k: dynamic) -> dynamic
  {
      return (cpp_assign( ((data[k] < 0)) ? k : data[k], "=", find(data[k])));
    }
  func unite(x: dynamic, y: dynamic) -> dynamic
  {
      x = find(x);
      y = find(y);
      if ((x == y))
      {
        return;
      }
      if ((data[x] > data[y]))
      {
        swap(x, y);
      }
      data[x] += data[y];
      data[y] = x;
    }
}

var g: dynamic = cpp_array(100001);

var leftt: dynamic = cpp_array(100001);

var rightt: dynamic = cpp_array(100001);

var just: dynamic = cpp_array(100001);

var gg: dynamic = cpp_array(100001);

var deg: dynamic = cpp_array(100001);

func rec(idx: dynamic, back: dynamic = -1) -> dynamic
{
  if ((rightt[idx][0] == INF))
  {
    {
      var i: dynamic = 0;
      while ((i < g[idx].size()))
      {
        var to: dynamic = cpp_uninitialized();
        var rev: dynamic = cpp_uninitialized();
        tie(to, rev) = g[idx][i];
        if ((i == back))
        {
          i += 1;
          continue;
        }
        just[idx][i] = (rec(to, rev) + 1);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < g[idx].size()))
      {
        leftt[idx][(i + 1)] = max(leftt[idx][i], just[idx][i]);
        i += 1;
      }
    }
    {
      var i: dynamic = (cpp_cast(g[idx].size()) - 1);
      while ((i >= 0))
      {
        rightt[idx][i] = max(rightt[idx][(i + 1)], just[idx][i]);
        i -= 1;
      }
    }
  }
  if ((back == -1))
  {
    return (rightt[idx][0]);
  }
  return (max(leftt[idx][back], rightt[idx][(back + 1)]));
}

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  var edges: dynamic = cpp_uninitialized();
  var arcs: dynamic = cpp_uninitialized();
  scanf("%d %d", (&N), (&M));
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      read(x, y, t);
      x -= 1;
      y -= 1;
      if ((t == 1))
      {
        arcs.emplace_back(x, y);
        g[x].emplace_back(y, -1);
      } else
      {
        if ((uf.find(x) == uf.find(y)))
        {
          write("Infinite", "\n");
          return (0);
        }
        uf.unite(x, y);
        edges.emplace_back(x, y);
        g[y].emplace_back(x, g[x].size());
        g[x].emplace_back(y, (g[y].size() - 1));
      }
      i += 1;
    }
  }
  {
    for (var e: dynamic in arcs)
    {
      gg[uf.find(e.first)].push_back(uf.find(e.second));
      deg[uf.find(e.second)] += 1;
    }
    var order: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < N))
      {
        if ((deg[i] == 0))
        {
          order.push_back(i);
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < order.size()))
      {
        for (var e: dynamic in gg[order[i]])
        {
          if ((cpp_update(deg[e], "--") == 0))
          {
            order.push_back(e);
          }
        }
        i += 1;
      }
    }
    if ((order.size() != N))
    {
      write("Infinite", "\n");
      return (0);
    }
  }
  var ret: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      var sz: dynamic = g[i].size();
      just[i].assign(sz, INF);
      leftt[i].assign((sz + 1), INF);
      rightt[i].assign((sz + 1), INF);
      leftt[i][0] = cpp_assign(rightt[i][sz], "=", 0);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      ret = max(ret, rec(i));
      i += 1;
    }
  }
  write(ret, "\n");
}
