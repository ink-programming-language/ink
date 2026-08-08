// Translated from solution.cpp.

class Graph
{
  var V: dynamic;
  var adj: dynamic;
}

func Graph(V: dynamic)
{
  this->V = V;
  adj = cpp_new();
}

func addEdge(v: dynamic, w: dynamic)
{
  adj[v].push_back(w);
}

func isCyclicUtil(v: dynamic, visited: dynamic, recStack: dynamic)
{
  if ((visited[v] == false))
  {
    visited[v] = true;
    recStack[v] = true;
    var i: dynamic;
    {
      i = adj[v].begin();
      while ((i != adj[v].end()))
      {
        if (((!visited[(*i)]) && isCyclicUtil((*i), visited, recStack)))
        {
          return true;
        } else if (recStack[(*i)])
        {
          return true;
        }
        i += 1;
      }
    }
  }
  recStack[v] = false;
  return false;
}

func isCyclic()
{
  var visited = cpp_new();
  var recStack = cpp_new();
  {
    var i = 0;
    while ((i < V))
    {
      visited[i] = false;
      recStack[i] = false;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < V))
    {
      if (isCyclicUtil(i, visited, recStack))
      {
        return true;
      }
      i += 1;
    }
  }
  return false;
}

func main()
{
  var V: dynamic;
  var E: dynamic;
  var s: dynamic;
  var t: dynamic;
  read(V, E);
  {
    var i = 0;
    while ((i < E))
    {
      read(s, t);
      g.addEdge(s, t);
      i += 1;
    }
  }
  if (g.isCyclic())
  {
    write(1, "\n");
  } else
  {
    write(0, "\n");
  }
  return 0;
}
