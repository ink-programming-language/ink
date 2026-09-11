// Translated from solution.cpp.

class Graph
{
  func Graph(n: dynamic) -> dynamic
  {
      self->e = cpp_construct();
      self->g = cpp_construct(n);
      self->d = cpp_construct(n);
      self->cur = cpp_construct(n);
    }
  var e: dynamic = cpp_uninitialized();
  var g: dynamic = cpp_uninitialized();
  func AddEdge(x: dynamic, y: dynamic, z: dynamic) -> dynamic
  {
      g[x].push_back(e.size());
      e.push_back([y, z]);
      g[y].push_back(e.size());
      e.push_back([x, 0]);
    }
  func MaxFlow(s: dynamic, t: dynamic) -> dynamic
  {
      s = s;
      t = t;
      var f: dynamic = 0;
      while (bfs())
      {
        fill(cur.begin(), cur.end(), 0);
        f += dfs(s, numeric_limits().max());
      }
      return f;
    }
  var d: dynamic = cpp_uninitialized();
  var cur: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  func bfs() -> dynamic
  {
      var q: dynamic = cpp_array(1000005);
      var l: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      fill(d.begin(), d.end(), -1);
      q[cpp_assign(l, "=", cpp_assign(r, "=", 1))] = s;
      d[s] = 0;
      while ((l <= r))
      {
        var x: dynamic = q[cpp_update(l, "++")];
        for (var i: dynamic in g[x])
        {
          if ((e[i].w && (!(~d[e[i].v]))))
          {
            d[cpp_assign(q[cpp_update(r, "++")], "=", e[i].v)] = (d[x] + 1);
          }
        }
      }
      return (~d[t]);
    }
  func dfs(x: dynamic, flow: dynamic) -> dynamic
  {
      if (((x == t) || (!flow)))
      {
        return flow;
      }
      var used: dynamic = 0;
      {
        while ((cur[x] < g[x].size()))
        {
          var i: dynamic = g[x][cur[x]];
          var y: dynamic = e[i].v;
          if (((!e[i].w) || (d[y] != (d[x] + 1))))
          {
            cur[x] += 1;
            continue;
          }
          var f: dynamic = dfs(y, min((flow - used), e[i].w));
          e[i].w -= f;
          e[(i ^ 1)].w += f;
          used += f;
          if ((flow == used))
          {
            return used;
          }
          cur[x] += 1;
        }
      }
      if ((!used))
      {
        d[x] = -1;
      }
      return used;
    }
}

var g: dynamic = cpp_uninitialized();

class Node
{
  var ls: dynamic = cpp_uninitialized();
  var rs: dynamic = cpp_uninitialized();
}

var t: dynamic = cpp_array(666666);

var cnt: dynamic = cpp_uninitialized();

var id: dynamic = cpp_uninitialized();

func build(o: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  id[PII(l, r)] = cpp_assign(o, "=", cpp_update(cnt, "++"));
  t[o].ls = cpp_assign(t[o].rs, "=", 0);
  if ((l == r))
  {
    return;
  }
  var m: dynamic = ((l + r) >> 1);
  build(t[o].ls, l, m);
  build(t[o].rs, (m + 1), r);
}

var vis: dynamic = cpp_array(666666);

func link(o: dynamic) -> dynamic
{
  if (vis[o])
  {
    return;
  }
  vis[o] = true;
  if (t[o].ls)
  {
    g->AddEdge(o, t[o].ls, INT_MAX);
    link(t[o].ls);
  }
  if (t[o].rs)
  {
    g->AddEdge(o, t[o].rs, INT_MAX);
    link(t[o].rs);
  }
}

func update(o: dynamic, l: dynamic, r: dynamic, x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  if (((x <= l) && (r <= y)))
  {
    if (z)
    {
      o = 0;
    } else
    {
      assert((!o));
      o = id[PII(l, r)];
    }
    return;
  }
  t[cpp_update(cnt, "++")] = t[o];
  o = cnt;
  var m: dynamic = ((l + r) >> 1);
  if ((x <= m))
  {
    update(t[o].ls, l, m, x, y, z);
  }
  if ((y > m))
  {
    update(t[o].rs, (m + 1), r, x, y, z);
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var add: dynamic = cpp_construct((n + 1));
  var del: dynamic = cpp_construct((n + 1));
  while (cpp_update(m, "--"))
  {
    var x1: dynamic = cpp_uninitialized();
    var y1: dynamic = cpp_uninitialized();
    var x2: dynamic = cpp_uninitialized();
    var y2: dynamic = cpp_uninitialized();
    read(x1, y1, x2, y2);
    x1 -= 1;
    add[x1].emplace_back(y1, y2);
    del[x2].emplace_back(y1, y2);
  }
  var root: dynamic = cpp_construct(1);
  build(root[0].first, 1, n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((add[i].empty() && del[i].empty()))
      {
        root.back().second += 1;
        i += 1;
        continue;
      }
      root.emplace_back(root.back().first, 1);
      for (var __cpp_item_1: dynamic in del[i])
      {
        var (l, r): dynamic = __cpp_item_1;
        update(root.back().first, 1, n, l, r, 0);
      }
      for (var __cpp_item_2: dynamic in add[i])
      {
        var (l, r): dynamic = __cpp_item_2;
        update(root.back().first, 1, n, l, r, 1);
      }
      i += 1;
    }
  }
  var s: dynamic = 0;
  var t: dynamic = (cnt + 1);
  g = cpp_new((cnt + 2));
  for (var __cpp_item_3: dynamic in root)
  {
    var (r, x): dynamic = __cpp_item_3;
    g->AddEdge(s, r, x);
    link(r);
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      g->AddEdge(id[PII(i, i)], t, 1);
      i += 1;
    }
  }
  write(g->MaxFlow(s, t), "\n");
  return 0;
}
