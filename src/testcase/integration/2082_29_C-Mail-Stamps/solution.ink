// Translated from solution.cpp.

var MAX = (1000000 + 5);

var n: dynamic;

var visited: dynamic;

var V: dynamic;

var Q: dynamic;

func Input()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      V[x].push_back(y);
      V[y].push_back(x);
      i += 1;
    }
  }
}

func DFS(s: dynamic)
{
  Q.push(s);
  visited[s] = true;
  {
    var i = 0;
    while ((i < V[s].size()))
    {
      var t = V[s][i];
      if ((visited[t] == false))
      {
        DFS(t);
      }
      i += 1;
    }
  }
}

func Solve()
{
  var temp = 0;
  var t = 0;
  var s = 0;
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

func main()
{
  Input();
  Solve();
  return 0;
}
