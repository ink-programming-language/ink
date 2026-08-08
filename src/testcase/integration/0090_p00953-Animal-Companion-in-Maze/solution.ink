// Translated from solution.cpp.

var INF = (1 << 29);

class UnionFind
{
  var data: dynamic;
  func UnionFind(sz: dynamic)
  {
      data.assign(sz, -1);
    }
  func find(k: dynamic)
  {
      return (cpp_assign(if ((data[k] < 0)) k else data[k], "=", find(data[k])));
    }
  func unite(x: dynamic, y: dynamic)
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

var g = cpp_array(100001);

var leftt = cpp_array(100001);

var rightt = cpp_array(100001);

var just = cpp_array(100001);

var gg = cpp_array(100001);

var deg = cpp_array(100001);

func rec(idx: dynamic, back: dynamic = -1)
{
  if ((rightt[idx][0] == INF))
  {
    {
      var i = 0;
      while ((i < g[idx].size()))
      {
        var to: dynamic;
        var rev: dynamic;
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
      var i = 0;
      while ((i < g[idx].size()))
      {
        leftt[idx][(i + 1)] = max(leftt[idx][i], just[idx][i]);
        i += 1;
      }
    }
    {
      var i = (cpp_cast(g[idx].size()) - 1);
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

func main()
{
  var N: dynamic;
  var M: dynamic;
  var edges: dynamic;
  var arcs: dynamic;
  scanf("%d %d", (&N), (&M));
  {
    var i = 0;
    while ((i < M))
    {
      var x: dynamic;
      var y: dynamic;
      var t: dynamic;
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
    for (var e in arcs)
    {
      gg[uf.find(e.first)].push_back(uf.find(e.second));
      deg[uf.find(e.second)] += 1;
    }
    var order: dynamic;
    {
      var i = 0;
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
      var i = 0;
      while ((i < order.size()))
      {
        for (var e in gg[order[i]])
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
  var ret = 0;
  {
    var i = 0;
    while ((i < N))
    {
      var sz = g[i].size();
      just[i].assign(sz, INF);
      leftt[i].assign((sz + 1), INF);
      rightt[i].assign((sz + 1), INF);
      leftt[i][0] = cpp_assign(rightt[i][sz], "=", 0);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      ret = max(ret, rec(i));
      i += 1;
    }
  }
  write(ret, "\n");
}
