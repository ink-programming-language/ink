// Translated from solution.cpp.

var MAX = 100000;

var NIL = -1;

var n: dynamic;

var G = cpp_array(MAX);

var color = cpp_array(MAX);

func dfs(r: dynamic, c: dynamic)
{
  var S: dynamic;
  S.push(r);
  color[r] = c;
  while ((!S.empty()))
  {
    var u = S.top();
    S.pop();
    var i: dynamic;
    {
      i = 0;
      while ((i < G[u].size()))
      {
        var v: dynamic;
        v = G[u][i];
        if ((color[v] == NIL))
        {
          color[v] = c;
          S.push(v);
        }
        i += 1;
      }
    }
  }
}

func ass()
{
  var id = 1;
  var i: dynamic;
  var u: dynamic;
  {
    i = 0;
    while ((i < n))
    {
      color[i] = NIL;
      i += 1;
    }
  }
  {
    u = 0;
    while ((u < n))
    {
      if ((color[u] == NIL))
      {
        dfs(u, cpp_update(id, "++"));
      }
      u += 1;
    }
  }
}

func main()
{
  var s: dynamic;
  var t: dynamic;
  var m: dynamic;
  var q: dynamic;
  read(n, m);
  var i: dynamic;
  {
    i = 0;
    while ((i < m))
    {
      read(s, t);
      G[s].push_back(t);
      G[t].push_back(s);
      i += 1;
    }
  }
  ass();
  read(q);
  {
    i = 0;
    while ((i < q))
    {
      read(s, t);
      if ((color[s] == color[t]))
      {
        write("yes", "\n");
      } else
      {
        write("no", "\n");
      }
      i += 1;
    }
  }
  return 0;
}
