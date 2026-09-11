// Translated from solution.cpp.

func power(x: dynamic, y: dynamic) -> dynamic
{
  var res: dynamic = 1;
  x = x;
  while ((y > 0))
  {
    if ((y & 1))
    {
      res = ((res * x));
    }
    y = (y >> 1);
    x = ((x * x));
  }
  return res;
}

func logtwo(n: dynamic) -> dynamic
{
  if ((n == 1))
  {
    return 0;
  }
  return (logtwo((n / 2)) + 1);
}

var adj: dynamic = cpp_array(105, 105);

var vis: dynamic = cpp_array(105, 105);

func dfs(par: dynamic, col: dynamic) -> dynamic
{
  for (var child: dynamic in adj[par][col])
  {
    if (vis[child][col])
    {
      continue;
    }
    vis[child][col] = vis[par][col];
    dfs(child, col);
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    var N: dynamic = cpp_uninitialized();
    var M: dynamic = cpp_uninitialized();
    read(N, M);
    {
      var i: dynamic = 0;
      while ((i < M))
      {
        var a: dynamic = cpp_uninitialized();
        var b: dynamic = cpp_uninitialized();
        var c: dynamic = cpp_uninitialized();
        read(a, b, c);
        a -= 1;
        b -= 1;
        adj[a][c].push_back(b);
        adj[b][c].push_back(a);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < 105))
      {
        var c: dynamic = 1;
        {
          var j: dynamic = 0;
          while ((j < N))
          {
            if (vis[j][i])
            {
              j += 1;
              continue;
            }
            vis[j][i] = cpp_update(c, "++");
            dfs(j, i);
            j += 1;
          }
        }
        i += 1;
      }
    }
    var Q: dynamic = cpp_uninitialized();
    read(Q);
    while (cpp_update(Q, "--"))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      var ans: dynamic = 0;
      x -= 1;
      y -= 1;
      {
        var i: dynamic = 0;
        while ((i < 105))
        {
          if ((vis[x][i] && (vis[x][i] == vis[y][i])))
          {
            ans += 1;
          }
          i += 1;
        }
      }
      write(ans, "\n");
    }
  }
  write("Time : ", ((1000 * cpp_cast(clock())) / cpp_cast(CLOCKS_PER_SEC)), "ms\n");
}
