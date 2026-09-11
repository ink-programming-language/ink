// Translated from solution.cpp.

var answer: dynamic = true;

func dfs(visited: dynamic, g: dynamic, vertex: dynamic, oddity: dynamic) -> dynamic
{
  visited[vertex] = oddity;
  {
    var i: dynamic = 0;
    while ((i < g[vertex].size()))
    {
      if ((visited[g[vertex][i]] == 0))
      {
        dfs(visited, g, g[vertex][i],  ((oddity == 1)) ? -1 : 1);
      } else if ((visited[g[vertex][i]] == visited[vertex]))
      {
        answer = false;
        return;
      }
      i += 1;
    }
  }
  return;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(x, y);
      x -= 1;
      y -= 1;
      g[x].push_back(y);
      g[y].push_back(x);
      edges[i].first = x;
      edges[i].second = y;
      i += 1;
    }
  }
  var visited: dynamic = cpp_construct(n, 0);
  dfs(visited, g, 0, 1);
  if ((answer == false))
  {
    write("NO", "\n");
  } else
  {
    write("YES", "\n");
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        if ((visited[edges[i].first] == 1))
        {
          write(1);
        } else
        {
          write(0);
        }
        i += 1;
      }
    }
    write("\n");
  }
  return 0;
}
