// Translated from solution.cpp.

var ll = dynamic;

var pb = cpp_expression("#include");

var N = 210;

var mod = (1e9 + 7);

var n: dynamic;

var inv = cpp_array(N);

var dp = cpp_array(N, N);

var lca = cpp_array(N, N);

var dep = cpp_array(N);

var g = cpp_array(N);

var s = cpp_array(N);

func dfs(cur: dynamic, fa: dynamic)
{
  s[cur] = [cur];
  dep[cur] = if (((fa >= 0))) (dep[fa] + 1) else 1;
  for (var child in g[cur])
  {
    if ((child == fa))
    {
      continue;
    }
    dfs(child, cur);
    for (var x in s[child])
    {
      for (var y in s[cur])
      {
        lca[x][y] = cpp_assign(lca[y][x], "=", cur);
      }
    }
    for (var x in s[child])
    {
      s[cur].pb(x);
    }
  }
}

func add(a: dynamic, b: dynamic)
{
  a += b;
  if ((a >= mod))
  {
    a -= mod;
  }
}

func main()
{
  read(n);
  inv[0] = cpp_assign(inv[1], "=", 1);
  {
    var i = 2;
    while ((i <= n))
    {
      inv[i] = ((cpp_cast(inv[(mod % i)]) * ((mod - (mod / i)))) % mod);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= n))
    {
      dp[i][0] = 0;
      dp[0][i] = 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          dp[i][j] = (((ll)((dp[(i - 1)][j] + dp[i][(j - 1)])) * inv[2]) % mod);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var u: dynamic;
  var v: dynamic;
  {
    var i = 1;
    while ((i < n))
    {
      read(u, v);
      u -= 1;
      v -= 1;
      g[u].pb(v);
      g[v].pb(u);
      i += 1;
    }
  }
  var ans = 0;
  {
    var rt = 0;
    while ((rt < n))
    {
      dfs(rt, -1);
      {
        var x = 0;
        while ((x < n))
        {
          {
            var y = (x + 1);
            while ((y < n))
            {
              add(ans, dp[(dep[y] - dep[lca[x][y]])][(dep[x] - dep[lca[x][y]])]);
              y += 1;
            }
          }
          x += 1;
        }
      }
      rt += 1;
    }
  }
  write((((cpp_cast(ans) * inv[n]) % mod)), cpp_char("\n"));
}
