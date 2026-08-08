// Translated from solution.cpp.

var rng = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var N = (2e5 + 20);

var g = cpp_array(N);

var edges: dynamic;

var bal: dynamic;

var ans: dynamic;

var vis = cpp_array(N);

var unvis: dynamic;

var p = cpp_array(N);

var p2 = cpp_array(N);

var cnt = cpp_array(N);

var sz = cpp_array(N);

func root(v: dynamic)
{
  if ((p[v] == v))
  {
    return v;
  }
  return cpp_assign(p[v], "=", root(p[v]));
}

func merge(a: dynamic, b: dynamic)
{
  a = root(a);
  b = root(b);
  if ((a == b))
  {
    return;
  }
  p[b] = a;
  sz[a] += sz[b];
}

func root2(v: dynamic)
{
  if ((p2[v] == v))
  {
    return v;
  }
  return cpp_assign(p2[v], "=", root2(p2[v]));
}

func merge2(a: dynamic, b: dynamic)
{
  a = root2(a);
  b = root2(b);
  if ((a == b))
  {
    return;
  }
  p2[b] = a;
}

var q: dynamic;

func bfs(s: dynamic)
{
  q.push(s);
  vis[s] = true;
  unvis.erase(s);
  while ((!q.empty()))
  {
    var v = q.front();
    q.pop();
    for (var x in g[v])
    {
      if ((!vis[x]))
      {
        unvis.erase(x);
      }
    }
    for (var x in unvis)
    {
      merge(v, x);
      q.push(x);
      vis[x] = true;
    }
    unvis.clear();
    for (var x in g[v])
    {
      if ((!vis[x]))
      {
        unvis.insert(x);
      }
    }
  }
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < m))
    {
      var u: dynamic;
      var v: dynamic;
      var w: dynamic;
      read(u, v, w);
      u -= 1;
      v -= 1;
      g[v].push_back(u);
      g[u].push_back(v);
      edges.push_back([w, [u, v]]);
      bal ^= w;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      unvis.insert(i);
      p[i] = i;
      sz[i] = 1;
      p2[i] = i;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      if ((!vis[i]))
      {
        bfs(i);
      }
      i += 1;
    }
  }
  var ok = false;
  {
    var i = 0;
    while ((i < m))
    {
      var (x, y) = edges[i].second;
      if ((root(x) == root(y)))
      {
        cnt[root(x)] += 1;
        bal = min(bal, edges[i].first);
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      var x = root(i);
      if (((((((sz[x] * (ll)((sz[x] - 1))) / 2) - cnt[x]) - sz[x]) + 1) > 0))
      {
        ok = true;
      }
      i += 1;
    }
  }
  sort(edges.begin(), edges.end());
  {
    var i = 0;
    while ((i < m))
    {
      var (x, y) = edges[i].second;
      if ((root(x) != root(y)))
      {
        ans += edges[i].first;
        merge(x, y);
        merge2(x, y);
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var (x, y) = edges[i].second;
      if ((root2(x) != root2(y)))
      {
        bal = min(bal, edges[i].first);
      }
      i += 1;
    }
  }
  if ((!ok))
  {
    ans += bal;
  }
  write(ans);
}
