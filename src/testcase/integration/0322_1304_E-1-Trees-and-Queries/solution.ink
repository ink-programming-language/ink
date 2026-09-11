// Translated from solution.cpp.

var mod: dynamic = (1e9 + 7);

func add(a: dynamic, b: dynamic) -> dynamic
{
  a += b;
  if ((a >= mod))
  {
    a -= mod;
  }
  return a;
}

func sub(a: dynamic, b: dynamic) -> dynamic
{
  a -= b;
  if ((a < 0))
  {
    a += mod;
  }
  return a;
}

func mul(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_cast((((cpp_cast(a) * b) % mod)));
}

var adj: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var lvl: dynamic = cpp_uninitialized();

func DFSUtil(u: dynamic, p: dynamic) -> dynamic
{
  if ((u != 0))
  {
    lvl[u] = (lvl[p] + 1);
  }
  dp[u][0] = p;
  {
    var i: dynamic = (1);
    while ((i <= (20)))
    {
      dp[u][i] = dp[dp[u][(i - 1)]][(i - 1)];
      i += 1;
    }
  }
  for (var it: dynamic in adj[u])
  {
    if ((it != p))
    {
      DFSUtil(it, u);
    }
  }
}

func DFS() -> dynamic
{
  var V: dynamic = adj.size();
  lvl.assign(V, 0);
  DFSUtil(0, 0);
}

func lca(x: dynamic, y: dynamic) -> dynamic
{
  if ((x == y))
  {
    return 0;
  }
  if ((lvl[x] < lvl[y]))
  {
    swap(x, y);
  }
  var d: dynamic = (lvl[x] - lvl[y]);
  var x1: dynamic = x;
  {
    var i: dynamic = (0);
    while ((i <= (20)))
    {
      if ((((1 << i)) & d))
      {
        x1 = dp[x1][i];
      }
      i += 1;
    }
  }
  if ((x1 == y))
  {
    return d;
  }
  var xx: dynamic = x1;
  var yy: dynamic = y;
  {
    var i: dynamic = (20);
    while ((i >= (0)))
    {
      if ((dp[xx][i] != dp[yy][i]))
      {
        d += (2 * ((1 << i)));
        xx = dp[xx][i];
        yy = dp[yy][i];
      }
      i -= 1;
    }
  }
  d += 2;
  return d;
}

func query() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(a, b, x, y, k);
  x -= 1;
  y -= 1;
  a -= 1;
  b -= 1;
  var v1: dynamic = lca(x, y);
  var v2: dynamic = lca(x, a);
  var v3: dynamic = lca(x, b);
  var v4: dynamic = lca(y, a);
  var v5: dynamic = lca(y, b);
  if (((v1 <= k) && ((((k - v1)) % 2) == 0)))
  {
    return true;
  }
  if ((((((v2 + v5) + 1)) <= k) && ((((k - (((v2 + v5) + 1)))) % 2) == 0)))
  {
    return true;
  }
  if ((((((v3 + v4) + 1)) <= k) && ((((k - (((v3 + v4) + 1)))) % 2) == 0)))
  {
    return true;
  }
  return false;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    adj.resize((n + 1));
    {
      var i: dynamic = (1);
      while ((i <= ((n - 1))))
      {
        var p: dynamic = cpp_uninitialized();
        var q: dynamic = cpp_uninitialized();
        read(p, q);
        p -= 1;
        q -= 1;
        adj[p].push_back(q);
        adj[q].push_back(p);
        i += 1;
      }
    }
    dp.assign((n + 1), vector(21, 0));
    DFS();
    var m: dynamic = cpp_uninitialized();
    read(m);
    {
      var i: dynamic = (1);
      while ((i <= (m)))
      {
        if (query())
        {
          write("YES\n");
        } else
        {
          write("NO\n");
        }
        i += 1;
      }
    }
  }
  return 0;
}
