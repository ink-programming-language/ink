// Translated from solution.cpp.

var rng: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var N: dynamic = (2e5 + 20);

var g: dynamic = cpp_array(N);

var edges: dynamic = cpp_uninitialized();

var bal: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(N);

var unvis: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(N);

var p2: dynamic = cpp_array(N);

var cnt: dynamic = cpp_array(N);

var sz: dynamic = cpp_array(N);

func root(v: dynamic) -> dynamic
{
  if ((p[v] == v))
  {
    return v;
  }
  return cpp_assign(p[v], "=", root(p[v]));
}

func merge(a: dynamic, b: dynamic) -> dynamic
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

func root2(v: dynamic) -> dynamic
{
  if ((p2[v] == v))
  {
    return v;
  }
  return cpp_assign(p2[v], "=", root2(p2[v]));
}

func merge2(a: dynamic, b: dynamic) -> dynamic
{
  a = root2(a);
  b = root2(b);
  if ((a == b))
  {
    return;
  }
  p2[b] = a;
}

var q: dynamic = cpp_uninitialized();

func bfs(s: dynamic) -> dynamic
{
  q.push(s);
  vis[s] = true;
  unvis.erase(s);
  while ((!q.empty()))
  {
    var v: dynamic = q.front();
    q.pop();
    for (var x: dynamic in g[v])
    {
      if ((!vis[x]))
      {
        unvis.erase(x);
      }
    }
    for (var x: dynamic in unvis)
    {
      merge(v, x);
      q.push(x);
      vis[x] = true;
    }
    unvis.clear();
    for (var x: dynamic in g[v])
    {
      if ((!vis[x]))
      {
        unvis.insert(x);
      }
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      var w: dynamic = cpp_uninitialized();
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
    var i: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((!vis[i]))
      {
        bfs(i);
      }
      i += 1;
    }
  }
  var ok: dynamic = false;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var (x, y): dynamic = edges[i].second;
      if ((root(x) == root(y)))
      {
        cnt[root(x)] += 1;
        bal = min(bal, edges[i].first);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = root(i);
      if (((((((sz[x] * (ll)((sz[x] - 1))) / 2) - cnt[x]) - sz[x]) + 1) > 0))
      {
        ok = true;
      }
      i += 1;
    }
  }
  sort(edges.begin(), edges.end());
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var (x, y): dynamic = edges[i].second;
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
    var i: dynamic = 0;
    while ((i < m))
    {
      var (x, y): dynamic = edges[i].second;
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
