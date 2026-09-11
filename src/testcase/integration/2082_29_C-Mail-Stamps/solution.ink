// Translated from solution.cpp.

var MAX: dynamic = (1000000 + 5);

var n: dynamic = cpp_uninitialized();

var visited: dynamic = cpp_uninitialized();

var V: dynamic = cpp_uninitialized();

var Q: dynamic = cpp_uninitialized();

func Input() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      V[x].push_back(y);
      V[y].push_back(x);
      i += 1;
    }
  }
}

func DFS(s: dynamic) -> dynamic
{
  Q.push(s);
  visited[s] = true;
  {
    var i: dynamic = 0;
    while ((i < V[s].size()))
    {
      var t: dynamic = V[s][i];
      if ((visited[t] == false))
      {
        DFS(t);
      }
      i += 1;
    }
  }
}

func Solve() -> dynamic
{
  var temp: dynamic = 0;
  var t: dynamic = 0;
  var s: dynamic = 0;
  {
    typeof(V.begin()) = V.begin();
    while ((i != V.end()))
    {
      temp = i->first;
      s = i->second.size();
      if ((i->second.size() == 1))
      {
        t = i->first;
      }
      i += 1;
    }
  }
  DFS(t);
  while ((!Q.empty()))
  {
    write(Q.front(), " ");
    Q.pop();
  }
}

func main() -> dynamic
{
  Input();
  Solve();
  return 0;
}
