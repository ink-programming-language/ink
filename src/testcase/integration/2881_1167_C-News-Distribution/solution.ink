// Translated from solution.cpp.

var visited: dynamic = [];

func dfs(grid: dynamic, start: dynamic, visited: dynamic, cnt: dynamic, comp: dynamic) -> dynamic
{
  cnt += 1;
  visited[start] = 1;
  comp.push_back(start);
  if ((grid[start].size() == 0))
  {
    return;
  }
  {
    var i: dynamic = 0;
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

func main() -> dynamic
{
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    read(n, m);
    var grid: dynamic = cpp_array(n);
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        var size: dynamic = cpp_uninitialized();
        read(size);
        var v: dynamic = cpp_uninitialized();
        if ((size == 0))
        {
          i += 1;
          continue;
        }
        {
          var j: dynamic = 0;
          while ((j < size))
          {
            var x: dynamic = cpp_uninitialized();
            read(x);
            v.push_back((x - 1));
            j += 1;
          }
        }
        {
          var k: dynamic = 0;
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
    var ans: dynamic = cpp_array(n);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if ((!visited[i]))
        {
          var cnt: dynamic = 0;
          var comp: dynamic = cpp_uninitialized();
          dfs(grid, i, visited, cnt, comp);
          for (var x: dynamic in comp)
          {
            ans[x] = cnt;
          }
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        write(ans[i], " ");
        i += 1;
      }
    }
  }
}
