// Translated from solution.cpp.

var answer = true;

func dfs(visited: dynamic, g: dynamic, vertex: dynamic, oddity: dynamic)
{
  visited[vertex] = oddity;
  {
    var i = 0;
    while ((i < g[vertex].size()))
    {
      if ((visited[g[vertex][i]] == 0))
      {
        dfs(visited, g, g[vertex][i], if ((oddity == 1)) -1 else 1);
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

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var x: dynamic;
  var y: dynamic;
  {
    var i = 0;
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
  var visited = cpp_construct(n, 0);
  dfs(visited, g, 0, 1);
  if ((answer == false))
  {
    write("NO", "\n");
  } else
  {
    write("YES", "\n");
    {
      var i = 0;
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
