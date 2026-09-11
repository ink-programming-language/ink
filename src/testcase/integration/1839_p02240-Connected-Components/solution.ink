// Translated from solution.cpp.

var MAX: dynamic = 100000;

var NIL: dynamic = -1;

var n: dynamic = cpp_uninitialized();

var G: dynamic = cpp_array(MAX);

var color: dynamic = cpp_array(MAX);

func dfs(r: dynamic, c: dynamic) -> dynamic
{
  var S: dynamic = cpp_uninitialized();
  S.push(r);
  color[r] = c;
  while ((!S.empty()))
  {
    var u: dynamic = S.top();
    S.pop();
    var i: dynamic = cpp_uninitialized();
    {
      i = 0;
      while ((i < G[u].size()))
      {
        var v: dynamic = cpp_uninitialized();
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

func ass() -> dynamic
{
  var id: dynamic = 1;
  var i: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
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

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(n, m);
  var i: dynamic = cpp_uninitialized();
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
