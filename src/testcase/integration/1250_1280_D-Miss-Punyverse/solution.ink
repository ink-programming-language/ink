// Translated from solution.cpp.

func minaj(x: dynamic, y: dynamic) -> dynamic
{
  x = ( ((x > y)) ? y : x);
}

func maxaj(x: dynamic, y: dynamic) -> dynamic
{
  x = ( ((x < y)) ? y : x);
}

var MAXN: dynamic = 3005;

var sz: dynamic = cpp_array(MAXN);

var e: dynamic = cpp_array(MAXN);

var a: dynamic = cpp_array(MAXN);

var dp: dynamic = cpp_array(MAXN, MAXN);

var ndp: dynamic = cpp_array(MAXN);

var m: dynamic = cpp_uninitialized();

func dfs(u: dynamic, v: dynamic) -> dynamic
{
  sz[v] = 1;
  dp[v][1] = pair(0, a[v]);
  for (var w: dynamic in e[v])
  {
    if ((u != w))
    {
      dfs(v, w);
      {
        var i: dynamic = 0;
        while ((i < ((sz[v] + sz[w]) + 1)))
        {
          ndp[i] = [-1, 0];
          i += 1;
        }
      }
      {
        var i: dynamic = 1;
        while ((i < (min(sz[v], m) + 1)))
        {
          {
            var j: dynamic = 1;
            while ((j < (min(sz[w], m) + 1)))
            {
              var v1: dynamic = dp[v][i].second;
              var v2: dynamic = dp[w][j].second;
              maxaj(ndp[((i + j) - 1)], pair((dp[v][i].first + dp[w][j].first), (v1 + v2)));
              maxaj(ndp[(i + j)], pair(((dp[v][i].first + dp[w][j].first) + ((v2 > 0))), v1));
              j += 1;
            }
          }
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < ((sz[v] + sz[w]) + 1)))
        {
          dp[v][i] = ndp[i];
          i += 1;
        }
      }
      sz[v] += sz[w];
    }
  }
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      a[i] -= x;
      a[i] *= -1;
      e[i].clear();
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      u -= 1;
      v -= 1;
      e[u].push_back(v);
      e[v].push_back(u);
      i += 1;
    }
  }
  dfs(0, 0);
  write((dp[0][m].first + ((dp[0][m].second > 0))), "\n");
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
