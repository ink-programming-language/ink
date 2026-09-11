// Translated from solution.cpp.

var adj: dynamic = cpp_array(2505);

var visited: dynamic = cpp_array(2505);

var a_points: dynamic = cpp_uninitialized();

var tin: dynamic = cpp_array(2505);

var low: dynamic = cpp_array(2505);

var timer: dynamic = cpp_uninitialized();

func dfs(u: dynamic, p: dynamic) -> dynamic
{
  visited[u] = true;
  tin[u] = cpp_assign(low[u], "=", cpp_update(timer, "++"));
  var child: dynamic = 0;
  for (var v: dynamic in adj[u])
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  timer = 0;
  memset(visited, false, cpp_sizeof((visited)));
  memset(tin, 0, cpp_sizeof((tin)));
  memset(low, 0, cpp_sizeof((low)));
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var c: dynamic = cpp_uninitialized();
  var mat: dynamic = cpp_array(m, n);
  var arr: dynamic = cpp_array(m, n);
  var dx: dynamic = [1, -1, 0, 0];
  var dy: dynamic = [0, 0, -1, 1];
  var cnt: dynamic = 1;
  var total: dynamic = 0;
  var tx: dynamic = cpp_uninitialized();
  var ty: dynamic = cpp_uninitialized();
  var last: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          c = mat[i][j];
          if ((c == cpp_char("#")))
          {
            total += 1;
            last = arr[i][j];
            {
              var k: dynamic = 0;
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
