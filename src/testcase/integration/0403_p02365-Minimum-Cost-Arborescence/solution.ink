// Translated from solution.cpp.

class StronglyConnectedComponents
{
  var gg: dynamic = cpp_uninitialized();
  var rg: dynamic = cpp_uninitialized();
  var edges: dynamic = cpp_uninitialized();
  var comp: dynamic = cpp_uninitialized();
  var order: dynamic = cpp_uninitialized();
  var used: dynamic = cpp_uninitialized();
  func StronglyConnectedComponents(v: dynamic) -> dynamic
  {
      self->gg = cpp_construct(v);
      self->rg = cpp_construct(v);
      self->comp = cpp_construct(v, -1);
      self->used = cpp_construct(v, 0);
    }
  func add_edge(x: dynamic, y: dynamic) -> dynamic
  {
      gg[x].push_back(y);
      rg[y].push_back(x);
      edges.emplace_back(x, y);
    }
  func operator_index(k: dynamic) -> dynamic
  {
      return (comp[k]);
    }
  func dfs(idx: dynamic) -> dynamic
  {
      if (used[idx])
      {
        return;
      }
      used[idx] = true;
      for (var to: dynamic in gg[idx])
      {
        dfs(to);
      }
      order.push_back(idx);
    }
  func rdfs(idx: dynamic, cnt: dynamic) -> dynamic
  {
      if ((comp[idx] != -1))
      {
        return;
      }
      comp[idx] = cnt;
      for (var to: dynamic in rg[idx])
      {
        rdfs(to, cnt);
      }
    }
  func build(t: dynamic) -> dynamic
  {
      {
        var i: dynamic = 0;
        while ((i < gg.size()))
        {
          dfs(i);
          i += 1;
        }
      }
      reverse(begin(order), end(order));
      var cpp_ptr: dynamic = 0;
      for (var i: dynamic in order)
      {
        if ((comp[i] == -1))
        {
          rdfs(i, cpp_ptr);
          cpp_ptr += 1;
        }
      }
      t.resize(cpp_ptr);
      var connect: dynamic = cpp_uninitialized();
      for (var e: dynamic in edges)
      {
        var x: dynamic = comp[e.first];
        var y: dynamic = comp[e.second];
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

var INF: dynamic = (1 << 30);

class edge
{
  var to: dynamic = cpp_uninitialized();
  var cost: dynamic = cpp_uninitialized();
}

func MST_Arborescence(g: dynamic, start: dynamic, sum: dynamic = 0) -> dynamic
{
  var N: dynamic = cpp_cast(g.size());
  var rev: dynamic = cpp_construct(N, -1);
  {
    var idx: dynamic = 0;
    while ((idx < N))
    {
      for (var e: dynamic in g[idx])
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
    var idx: dynamic = 0;
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
  var renew: dynamic = cpp_uninitialized();
  scc.build(renew);
  if ((renew.size() == N))
  {
    return (sum);
  }
  var fixgraph: dynamic = cpp_construct(renew.size());
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      for (var e: dynamic in g[i])
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

func solve() -> dynamic
{
  var V: dynamic = cpp_uninitialized();
  var E: dynamic = cpp_uninitialized();
  var R: dynamic = cpp_uninitialized();
  read(V, E, R);
  while (cpp_update(E, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var c: dynamic = cpp_uninitialized();
    read(a, b, c);
    g[a].emplace_back([b, c]);
  }
  write(MST_Arborescence(g, R), "\n");
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  solve();
}
