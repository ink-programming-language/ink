// Translated from solution.cpp.

var eps: dynamic = 1e-7;

var inf: dynamic = 1000000010;

var INF: dynamic = 10000000000000010;

var mod: dynamic = 998244353;

var MAXN: dynamic = 300010;

var LOG: dynamic = 18;

class DSU
{
  var par: dynamic = cpp_array(MAXN);
  func DSU() -> dynamic
  {
      {
        var i: dynamic = 1;
        while ((i < MAXN))
        {
          par[i] = i;
          i += 1;
        }
      }
    }
  func get(x: dynamic) -> dynamic
  {
      if ((par[x] == x))
      {
        return x;
      }
      return cpp_assign(par[x], "=", get(par[x]));
    }
  func join(x: dynamic, y: dynamic) -> dynamic
  {
      par[get(x)] = get(y);
    }
}

var dsu: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var u: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var A: dynamic = cpp_array(MAXN);

var B: dynamic = cpp_array(MAXN);

var h: dynamic = cpp_array(MAXN);

var par: dynamic = cpp_array(LOG, MAXN);

var dp: dynamic = cpp_array(2, MAXN);

var G: dynamic = cpp_array(MAXN);

var vec: dynamic = cpp_array(MAXN);

func dfs1(node: dynamic, p: dynamic) -> dynamic
{
  h[node] = (h[p] + 1);
  par[node][0] = p;
  {
    var i: dynamic = 1;
    while ((i < LOG))
    {
      par[node][i] = par[par[node][(i - 1)]][(i - 1)];
      i += 1;
    }
  }
  for (var v: dynamic in G[node])
  {
    if ((v != p))
    {
      dfs1(v, node);
    }
  }
}

func Lca(x: dynamic, y: dynamic) -> dynamic
{
  if ((h[x] > h[y]))
  {
    swap(x, y);
  }
  {
    var i: dynamic = 0;
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
    var i: dynamic = (LOG - 1);
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

func dfs2(node: dynamic, p: dynamic) -> dynamic
{
  for (var v: dynamic in G[node])
  {
    if ((v != p))
    {
      B[node] += dfs2(v, node);
    }
  }
  return B[node];
}

func powmod(a: dynamic, b: dynamic) -> dynamic
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

func inv(x: dynamic) -> dynamic
{
  return powmod(x, (mod - 2));
}

func dfs3(node: dynamic) -> dynamic
{
  for (var v: dynamic in G[node])
  {
    dfs3(v);
  }
  dp[node][0] = 1;
  for (var v: dynamic in G[node])
  {
    dp[node][0] = ((dp[node][0] * ((dp[v][0] + dp[v][1]))) % mod);
  }
  if (vec[node].empty())
  {
    for (var v: dynamic in G[node])
    {
      dp[node][1] = (((dp[node][1] + (dp[v][1] * inv((dp[v][0] + dp[v][1]))))) % mod);
    }
    dp[node][1] = ((dp[node][1] * dp[node][0]) % mod);
    return;
  }
  swap(dp[node][0], dp[node][1]);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n, k);
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= k))
    {
      if ((vec[i].size() > 0))
      {
        var v: dynamic = vec[i][0];
        {
          var j: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      G[i].clear();
      vec[i].clear();
      i += 1;
    }
  }
  {
    var i: dynamic = 2;
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
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (vec[i].size())
      {
        var shit: dynamic = vec[i][0];
        for (var j: dynamic in vec[i])
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
    var i: dynamic = 2;
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
