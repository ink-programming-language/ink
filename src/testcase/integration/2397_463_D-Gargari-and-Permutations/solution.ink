// Translated from solution.cpp.

var md = (1e9 + 7);

var n: dynamic;

var k: dynamic;

var ans: dynamic;

var mx: dynamic;

var edg = cpp_array(1010, 1010);

var dis = cpp_array(1001);

var v: dynamic;

var gr = cpp_array(100000);

var vis = cpp_array(1001);

func dfs(x: dynamic, l: dynamic)
{
  vis[x] = true;
  dis[x] = 0;
  {
    var i = 0;
    while ((i < gr[x].size()))
    {
      var u = gr[x][i];
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  read(n, k);
  var kk = k;
  while (cpp_update(k, "--"))
  {
    {
      var i = 1;
      while ((i <= n))
      {
        var x: dynamic;
        read(x);
        edg[x][k] = i;
        i += 1;
      }
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
          if ((i == j))
          {
            j += 1;
            continue;
          }
          var t = false;
          {
            var z = 0;
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
    var i = 1;
    while ((i <= n))
    {
      dfs(i, 1);
      memset(vis, 0, cpp_sizeof(vis));
      i += 1;
    }
  }
  write((mx + 1));
}
