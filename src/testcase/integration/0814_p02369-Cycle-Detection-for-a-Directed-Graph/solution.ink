// Translated from solution.cpp.

class Graph
{
  var V: dynamic = cpp_uninitialized();
  var adj: dynamic = cpp_uninitialized();
}

func Graph(V: dynamic) -> dynamic
{
  self->V = V;
  adj = cpp_new();
}

func addEdge(v: dynamic, w: dynamic) -> dynamic
{
  adj[v].push_back(w);
}

func isCyclicUtil(v: dynamic, visited: dynamic, recStack: dynamic) -> dynamic
{
  if ((visited[v] == false))
  {
    visited[v] = true;
    recStack[v] = true;
    var i: dynamic = cpp_uninitialized();
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

func isCyclic() -> dynamic
{
  var visited: dynamic = cpp_new();
  var recStack: dynamic = cpp_new();
  {
    var i: dynamic = 0;
    while ((i < V))
    {
      visited[i] = false;
      recStack[i] = false;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
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

func main() -> dynamic
{
  var V: dynamic = cpp_uninitialized();
  var E: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(V, E);
  {
    var i: dynamic = 0;
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
