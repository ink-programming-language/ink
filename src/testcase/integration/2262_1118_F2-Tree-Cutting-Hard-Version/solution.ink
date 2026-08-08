// Translated from solution.cpp.

var eps = 1e-7;

var inf = 1000000010;

var INF = 10000000000000010;

var mod = 998244353;

var MAXN = 300010;

var LOG = 18;

class DSU
{
  var par: dynamic = cpp_array(MAXN);
  func DSU()
  {
      {
        var i = 1;
        while ((i < MAXN))
        {
          par[i] = i;
          i += 1;
        }
      }
    }
  func get(x: dynamic)
  {
      if ((par[x] == x))
      {
        return x;
      }
      return cpp_assign(par[x], "=", get(par[x]));
    }
  func join(x: dynamic, y: dynamic)
  {
      par[get(x)] = get(y);
    }
}

var dsu: dynamic;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var u: dynamic;

var v: dynamic;

var x: dynamic;

var y: dynamic;

var t: dynamic;

var a: dynamic;

var b: dynamic;

var A = cpp_array(MAXN);

var B = cpp_array(MAXN);

var h = cpp_array(MAXN);

var par = cpp_array(LOG, MAXN);

var dp = cpp_array(2, MAXN);

var G = cpp_array(MAXN);

var vec = cpp_array(MAXN);

func dfs1(node: dynamic, p: dynamic)
{
  h[node] = (h[p] + 1);
  par[node][0] = p;
  {
    var i = 1;
    while ((i < LOG))
    {
      par[node][i] = par[par[node][(i - 1)]][(i - 1)];
      i += 1;
    }
  }
  for (var v in G[node])
  {
    if ((v != p))
    {
      dfs1(v, node);
    }
  }
}

func Lca(x: dynamic, y: dynamic)
{
  if ((h[x] > h[y]))
  {
    swap(x, y);
  }
  {
    var i = 0;
    while ((i < LOG))
    {
      if ((((h[y] - h[x])) & ((1 << i))))
      {
        y = par[y][i];
      }
      i += 1;
    }
  }
  if ((x == y))
  {
    return x;
  }
  {
    var i = (LOG - 1);
    while ((i >= 0))
    {
      if ((par[x][i] != par[y][i]))
      {
        x = par[x][i];
        y = par[y][i];
      }
      i -= 1;
    }
  }
  return par[x][0];
}

func dfs2(node: dynamic, p: dynamic)
{
  for (var v in G[node])
  {
    if ((v != p))
    {
      B[node] += dfs2(v, node);
    }
  }
  return B[node];
}

func powmod(a: dynamic, b: dynamic)
{
  if ((!b))
  {
    return 1;
  }
  if ((b & 1))
  {
    return ((a * powmod(((a * a) % mod), (b >> 1))) % mod);
  }
  return powmod(((a * a) % mod), (b >> 1));
}

func inv(x: dynamic)
{
  return powmod(x, (mod - 2));
}

func dfs3(node: dynamic)
{
  for (var v in G[node])
  {
    dfs3(v);
  }
  dp[node][0] = 1;
  for (var v in G[node])
  {
    dp[node][0] = ((dp[node][0] * ((dp[v][0] + dp[v][1]))) % mod);
  }
  if (vec[node].empty())
  {
    for (var v in G[node])
    {
      dp[node][1] = (((dp[node][1] + (dp[v][1] * inv((dp[v][0] + dp[v][1]))))) % mod);
    }
    dp[node][1] = ((dp[node][1] * dp[node][0]) % mod);
    return;
  }
  swap(dp[node][0], dp[node][1]);
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n, k);
  {
    var i = 1;
    while ((i <= n))
    {
      read(A[i]);
      if (A[i])
      {
        B[i] += 1;
      }
      vec[A[i]].push_back(i);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      read(u, v);
      G[u].push_back(v);
      G[v].push_back(u);
      i += 1;
    }
  }
  dfs1(1, 1);
  {
    var i = 1;
    while ((i <= k))
    {
      if ((vec[i].size() > 0))
      {
        var v = vec[i][0];
        {
          var j = 1;
          while ((j < vec[i].size()))
          {
            v = Lca(v, vec[i][j]);
            j += 1;
          }
        }
        B[v] -= vec[i].size();
      }
      i += 1;
    }
  }
  dfs2(1, 1);
  {
    var i = 1;
    while ((i <= n))
    {
      G[i].clear();
      vec[i].clear();
      i += 1;
    }
  }
  {
    var i = 2;
    while ((i <= n))
    {
      if (B[i])
      {
        dsu.join(i, par[i][0]);
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if (A[i])
      {
        vec[dsu.get(i)].push_back(A[i]);
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if (vec[i].size())
      {
        var shit = vec[i][0];
        for (var j in vec[i])
        {
          if ((j != shit))
          {
            return cpp_comma(((cout << 0) << cpp_char("\n")), 0);
          }
        }
      }
      i += 1;
    }
  }
  {
    var i = 2;
    while ((i <= n))
    {
      if ((!B[i]))
      {
        G[dsu.get(par[i][0])].push_back(dsu.get(i));
      }
      i += 1;
    }
  }
  dfs3(1);
  write(dp[1][1], cpp_char("\n"));
  return 0;
}
