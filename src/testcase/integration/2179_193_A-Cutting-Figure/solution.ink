// Translated from solution.cpp.

var adj = cpp_array(2505);

var visited = cpp_array(2505);

var a_points: dynamic;

var tin = cpp_array(2505);

var low = cpp_array(2505);

var timer: dynamic;

func dfs(u: dynamic, p: dynamic)
{
  visited[u] = true;
  tin[u] = cpp_assign(low[u], "=", cpp_update(timer, "++"));
  var child = 0;
  for (var v in adj[u])
  {
    if ((p == v))
    {
      continue;
    } else if (visited[v])
    {
      low[u] = min(tin[v], low[u]);
    } else
    {
      child += 1;
      dfs(v, u);
      low[u] = min(low[u], low[v]);
      if (((low[v] >= tin[u]) && (p != -1)))
      {
        a_points.insert(u);
      }
    }
  }
  if (((child > 1) && (p == -1)))
  {
    a_points.insert(u);
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  timer = 0;
  memset(visited, false, cpp_sizeof((visited)));
  memset(tin, 0, cpp_sizeof((tin)));
  memset(low, 0, cpp_sizeof((low)));
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var c: dynamic;
  var mat = cpp_array(m, n);
  var arr = cpp_array(m, n);
  var dx = [1, -1, 0, 0];
  var dy = [0, 0, -1, 1];
  var cnt = 1;
  var total = 0;
  var tx: dynamic;
  var ty: dynamic;
  var last: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          read(c);
          mat[i][j] = c;
          arr[i][j] = cpp_update(cnt, "++");
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          c = mat[i][j];
          if ((c == cpp_char("#")))
          {
            total += 1;
            last = arr[i][j];
            {
              var k = 0;
              while ((k < 4))
              {
                tx = (i + dx[k]);
                ty = (j + dy[k]);
                if ((((((tx >= 0) && (tx < n)) && (ty >= 0)) && (ty < m)) && (mat[tx][ty] == cpp_char("#"))))
                {
                  adj[arr[i][j]].push_back(arr[tx][ty]);
                }
                k += 1;
              }
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((((last == -1) || (total == 1)) || (total == 2)))
  {
    write("-1\n");
  } else
  {
    dfs(last, -1);
    if (a_points.size())
    {
      write("1\n");
    } else
    {
      write("2\n");
    }
  }
  return 0;
}
