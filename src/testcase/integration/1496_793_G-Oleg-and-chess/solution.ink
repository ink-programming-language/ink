// Translated from solution.cpp.

class Graph
{
  func Graph(n: dynamic)
  {
      this->e = cpp_construct();
      this->g = cpp_construct(n);
      this->d = cpp_construct(n);
      this->cur = cpp_construct(n);
    }
  var e: dynamic;
  var g: dynamic;
  func AddEdge(x: dynamic, y: dynamic, z: dynamic)
  {
      g[x].push_back(e.size());
      e.push_back([y, z]);
      g[y].push_back(e.size());
      e.push_back([x, 0]);
    }
  func MaxFlow(s: dynamic, t: dynamic)
  {
      s = s;
      t = t;
      var f = 0;
      while (bfs())
      {
        fill(cur.begin(), cur.end(), 0);
        f += dfs(s, numeric_limits().max());
      }
      return f;
    }
  var d: dynamic;
  var cur: dynamic;
  var s: dynamic;
  var t: dynamic;
  func bfs()
  {
      var q = cpp_array(1000005);
      var l: dynamic;
      var r: dynamic;
      fill(d.begin(), d.end(), -1);
      q[cpp_assign(l, "=", cpp_assign(r, "=", 1))] = s;
      d[s] = 0;
      while ((l <= r))
      {
        var x = q[cpp_update(l, "++")];
        for (var i in g[x])
        {
          if ((e[i].w && (!(~d[e[i].v]))))
          {
            d[cpp_assign(q[cpp_update(r, "++")], "=", e[i].v)] = (d[x] + 1);
          }
        }
      }
      return (~d[t]);
    }
  func dfs(x: dynamic, flow: dynamic)
  {
      if (((x == t) || (!flow)))
      {
        return flow;
      }
      var used = 0;
      {
        while ((cur[x] < g[x].size()))
        {
          var i = g[x][cur[x]];
          var y = e[i].v;
          if (((!e[i].w) || (d[y] != (d[x] + 1))))
          {
            cur[x] += 1;
            continue;
          }
          var f = dfs(y, min((flow - used), e[i].w));
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

var g: dynamic;

class Node
{
  var ls: dynamic;
  var rs: dynamic;
}

var t = cpp_array(666666);

var cnt: dynamic;

var id: dynamic;

func build(o: dynamic, l: dynamic, r: dynamic)
{
  id[PII(l, r)] = cpp_assign(o, "=", cpp_update(cnt, "++"));
  t[o].ls = cpp_assign(t[o].rs, "=", 0);
  if ((l == r))
  {
    return;
  }
  var m = ((l + r) >> 1);
  build(t[o].ls, l, m);
  build(t[o].rs, (m + 1), r);
}

var vis = cpp_array(666666);

func link(o: dynamic)
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

func update(o: dynamic, l: dynamic, r: dynamic, x: dynamic, y: dynamic, z: dynamic)
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
  var m = ((l + r) >> 1);
  if ((x <= m))
  {
    update(t[o].ls, l, m, x, y, z);
  }
  if ((y > m))
  {
    update(t[o].rs, (m + 1), r, x, y, z);
  }
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var add = cpp_construct((n + 1));
  var del = cpp_construct((n + 1));
  while (cpp_update(m, "--"))
  {
    var x1: dynamic;
    var y1: dynamic;
    var x2: dynamic;
    var y2: dynamic;
    read(x1, y1, x2, y2);
    x1 -= 1;
    add[x1].emplace_back(y1, y2);
    del[x2].emplace_back(y1, y2);
  }
  var root = cpp_construct(1);
  build(root[0].first, 1, n);
  {
    var i = 0;
    while ((i < n))
    {
      if ((add[i].empty() && del[i].empty()))
      {
        root.back().second += 1;
        i += 1;
        continue;
      }
      root.emplace_back(root.back().first, 1);
      for (var __cpp_item_1 in del[i])
      {
        var (l, r) = __cpp_item_1;
        update(root.back().first, 1, n, l, r, 0);
      }
      for (var __cpp_item_2 in add[i])
      {
        var (l, r) = __cpp_item_2;
        update(root.back().first, 1, n, l, r, 1);
      }
      i += 1;
    }
  }
  var s = 0;
  var t = (cnt + 1);
  g = cpp_new((cnt + 2));
  for (var __cpp_item_3 in root)
  {
    var (r, x) = __cpp_item_3;
    g->AddEdge(s, r, x);
    link(r);
  }
  {
    var i = 1;
    while ((i <= n))
    {
      g->AddEdge(id[PII(i, i)], t, 1);
      i += 1;
    }
  }
  write(g->MaxFlow(s, t), "\n");
  return 0;
}
