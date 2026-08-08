// Translated from solution.cpp.

class StronglyConnectedComponents
{
  var gg: dynamic;
  var rg: dynamic;
  var edges: dynamic;
  var comp: dynamic;
  var order: dynamic;
  var used: dynamic;
  func StronglyConnectedComponents(v: dynamic)
  {
      this->gg = cpp_construct(v);
      this->rg = cpp_construct(v);
      this->comp = cpp_construct(v, -1);
      this->used = cpp_construct(v, 0);
    }
  func add_edge(x: dynamic, y: dynamic)
  {
      gg[x].push_back(y);
      rg[y].push_back(x);
      edges.emplace_back(x, y);
    }
  func operator_index(k: dynamic)
  {
      return (comp[k]);
    }
  func dfs(idx: dynamic)
  {
      if (used[idx])
      {
        return;
      }
      used[idx] = true;
      for (var to in gg[idx])
      {
        dfs(to);
      }
      order.push_back(idx);
    }
  func rdfs(idx: dynamic, cnt: dynamic)
  {
      if ((comp[idx] != -1))
      {
        return;
      }
      comp[idx] = cnt;
      for (var to in rg[idx])
      {
        rdfs(to, cnt);
      }
    }
  func build(t: dynamic)
  {
      {
        var i = 0;
        while ((i < gg.size()))
        {
          dfs(i);
          i += 1;
        }
      }
      reverse(begin(order), end(order));
      var ptr = 0;
      for (var i in order)
      {
        if ((comp[i] == -1))
        {
          rdfs(i, ptr);
          ptr += 1;
        }
      }
      t.resize(ptr);
      var connect: dynamic;
      for (var e in edges)
      {
        var x = comp[e.first];
        var y = comp[e.second];
        if ((x == y))
        {
          continue;
        }
        if (connect.count([x, y]))
        {
          continue;
        }
        t[x].push_back(y);
        connect.emplace(x, y);
      }
    }
}

var INF = (1 << 30);

class edge
{
  var to: dynamic;
  var cost: dynamic;
}

func MST_Arborescence(g: dynamic, start: dynamic, sum: dynamic = 0)
{
  var N = cpp_cast(g.size());
  var rev = cpp_construct(N, -1);
  {
    var idx = 0;
    while ((idx < N))
    {
      for (var e in g[idx])
      {
        if ((e.cost < weight[e.to]))
        {
          weight[e.to] = e.cost;
          rev[e.to] = idx;
        }
      }
      idx += 1;
    }
  }
  {
    var idx = 0;
    while ((idx < N))
    {
      if ((start == idx))
      {
        idx += 1;
        continue;
      }
      scc.add_edge(rev[idx], idx);
      sum += weight[idx];
      idx += 1;
    }
  }
  var renew: dynamic;
  scc.build(renew);
  if ((renew.size() == N))
  {
    return (sum);
  }
  var fixgraph = cpp_construct(renew.size());
  {
    var i = 0;
    while ((i < N))
    {
      for (var e in g[i])
      {
        if ((scc[i] == scc[e.to]))
        {
          continue;
        }
        fixgraph[scc[i]].emplace_back([scc[e.to], (e.cost - weight[e.to])]);
      }
      i += 1;
    }
  }
  return (MST_Arborescence(fixgraph, scc[start], sum));
}

func solve()
{
  var V: dynamic;
  var E: dynamic;
  var R: dynamic;
  read(V, E, R);
  while (cpp_update(E, "--"))
  {
    var a: dynamic;
    var b: dynamic;
    var c: dynamic;
    read(a, b, c);
    g[a].emplace_back([b, c]);
  }
  write(MST_Arborescence(g, R), "\n");
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  solve();
}
