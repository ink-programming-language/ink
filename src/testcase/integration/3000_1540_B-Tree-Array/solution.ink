// Translated from solution.cpp.

var ll: dynamic = dynamic;

var pb: dynamic = cpp_expression("#include");

var N: dynamic = 210;

var mod: dynamic = (1e9 + 7);

var n: dynamic = cpp_uninitialized();

var inv: dynamic = cpp_array(N);

var dp: dynamic = cpp_array(N, N);

var lca: dynamic = cpp_array(N, N);

var dep: dynamic = cpp_array(N);

var g: dynamic = cpp_array(N);

var s: dynamic = cpp_array(N);

func dfs(cur: dynamic, fa: dynamic) -> dynamic
{
  s[cur] = [cur];
  dep[cur] =  (((fa >= 0))) ? (dep[fa] + 1) : 1;
  for (var child: dynamic in g[cur])
  {
    if ((child == fa))
    {
      continue;
    }
    dfs(child, cur);
    for (var x: dynamic in s[child])
    {
      for (var y: dynamic in s[cur])
      {
        lca[x][y] = cpp_assign(lca[y][x], "=", cur);
      }
    }
    for (var x: dynamic in s[child])
    {
      s[cur].pb(x);
    }
  }
}

func add(a: dynamic, b: dynamic) -> dynamic
{
  a += b;
  if ((a >= mod))
  {
    a -= mod;
  }
}

func main() -> dynamic
{
  read(n);
  inv[0] = cpp_assign(inv[1], "=", 1);
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      inv[i] = ((cpp_cast(inv[(mod % i)]) * ((mod - (mod / i)))) % mod);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      dp[i][0] = 0;
      dp[0][i] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          dp[i][j] = (((ll)((dp[(i - 1)][j] + dp[i][(j - 1)])) * inv[2]) % mod);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
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
  var ans: dynamic = 0;
  {
    var rt: dynamic = 0;
    while ((rt < n))
    {
      dfs(rt, -1);
      {
        var x: dynamic = 0;
        while ((x < n))
        {
          {
            var y: dynamic = (x + 1);
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
