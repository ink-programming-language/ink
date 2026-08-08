// Translated from solution.cpp.

var visited = [];

func dfs(grid: dynamic, start: dynamic, visited: dynamic, cnt: dynamic, comp: dynamic)
{
  cnt += 1;
  visited[start] = 1;
  comp.push_back(start);
  if ((grid[start].size() == 0))
  {
    return;
  }
  {
    var i = 0;
    while ((i < grid[start].size()))
    {
      if ((visited[grid[start][i]] == 0))
      {
        dfs(grid, grid[start][i], visited, cnt, comp);
      }
      i += 1;
    }
  }
}

func main()
{
  var t = 1;
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var m: dynamic;
    read(n, m);
    var grid = cpp_array(n);
    {
      var i = 0;
      while ((i < m))
      {
        var size: dynamic;
        read(size);
        var v: dynamic;
        if ((size == 0))
        {
          i += 1;
          continue;
        }
        {
          var j = 0;
          while ((j < size))
          {
            var x: dynamic;
            read(x);
            v.push_back((x - 1));
            j += 1;
          }
        }
        {
          var k = 0;
          while ((k < (v.size() - 1)))
          {
            grid[v[k]].push_back(v[(k + 1)]);
            grid[v[(k + 1)]].push_back(v[k]);
            k += 1;
          }
        }
        i += 1;
      }
    }
    var ans = cpp_array(n);
    {
      var i = 0;
      while ((i < n))
      {
        if ((!visited[i]))
        {
          var cnt = 0;
          var comp: dynamic;
          dfs(grid, i, visited, cnt, comp);
          for (var x in comp)
          {
            ans[x] = cnt;
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < n))
      {
        write(ans[i], " ");
        i += 1;
      }
    }
  }
}
