// Translated from solution.cpp.

var md: dynamic = (1e9 + 7);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var mx: dynamic = cpp_uninitialized();

var edg: dynamic = cpp_array(1010, 1010);

var dis: dynamic = cpp_array(1001);

var v: dynamic = cpp_uninitialized();

var gr: dynamic = cpp_array(100000);

var vis: dynamic = cpp_array(1001);

func dfs(x: dynamic, l: dynamic) -> dynamic
{
  vis[x] = true;
  dis[x] = 0;
  {
    var i: dynamic = 0;
    while ((i < gr[x].size()))
    {
      var u: dynamic = gr[x][i];
      if ((vis[u] == false))
      {
        dfs(u, (l + 1));
      }
      dis[x] = max(dis[x], (dis[u] + 1));
      mx = max(mx, dis[x]);
      i += 1;
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  read(n, k);
  var kk: dynamic = k;
  while (cpp_update(k, "--"))
  {
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        var x: dynamic = cpp_uninitialized();
        read(x);
        edg[x][k] = i;
        i += 1;
      }
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
          if ((i == j))
          {
            j += 1;
            continue;
          }
          var t: dynamic = false;
          {
            var z: dynamic = 0;
            while ((z < kk))
            {
              if ((edg[i][z] > edg[j][z]))
              {
                t = true;
              }
              z += 1;
            }
          }
          if ((t == false))
          {
            gr[i].push_back(j);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      dfs(i, 1);
      memset(vis, 0, cpp_sizeof(vis));
      i += 1;
    }
  }
  write((mx + 1));
}
