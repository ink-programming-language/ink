// Translated from solution.cpp.

var mod = (1e9 + 7);

func add(a: dynamic, b: dynamic)
{
  a += b;
  if ((a >= mod))
  {
    a -= mod;
  }
  return a;
}

func sub(a: dynamic, b: dynamic)
{
  a -= b;
  if ((a < 0))
  {
    a += mod;
  }
  return a;
}

func mul(a: dynamic, b: dynamic)
{
  return cpp_cast((((cpp_cast(a) * b) % mod)));
}

var adj: dynamic;

var dp: dynamic;

var cnt: dynamic;

var lvl: dynamic;

func DFSUtil(u: dynamic, p: dynamic)
{
  if ((u != 0))
  {
    lvl[u] = (lvl[p] + 1);
  }
  dp[u][0] = p;
  {
    var i = (1);
    while ((i <= (20)))
    {
      dp[u][i] = dp[dp[u][(i - 1)]][(i - 1)];
      i += 1;
    }
  }
  for (var it in adj[u])
  {
    if ((it != p))
    {
      DFSUtil(it, u);
    }
  }
}

func DFS()
{
  var V = adj.size();
  lvl.assign(V, 0);
  DFSUtil(0, 0);
}

func lca(x: dynamic, y: dynamic)
{
  if ((x == y))
  {
    return 0;
  }
  if ((lvl[x] < lvl[y]))
  {
    swap(x, y);
  }
  var d = (lvl[x] - lvl[y]);
  var x1 = x;
  {
    var i = (0);
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
  var xx = x1;
  var yy = y;
  {
    var i = (20);
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

func query()
{
  var a: dynamic;
  var b: dynamic;
  var x: dynamic;
  var y: dynamic;
  var k: dynamic;
  read(a, b, x, y, k);
  x -= 1;
  y -= 1;
  a -= 1;
  b -= 1;
  var v1 = lca(x, y);
  var v2 = lca(x, a);
  var v3 = lca(x, b);
  var v4 = lca(y, a);
  var v5 = lca(y, b);
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t = 1;
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    adj.resize((n + 1));
    {
      var i = (1);
      while ((i <= ((n - 1))))
      {
        var p: dynamic;
        var q: dynamic;
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
    var m: dynamic;
    read(m);
    {
      var i = (1);
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
