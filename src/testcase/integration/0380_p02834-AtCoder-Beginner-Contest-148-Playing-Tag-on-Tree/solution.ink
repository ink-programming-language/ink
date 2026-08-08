// Translated from solution.cpp.

var n: dynamic;

var u: dynamic;

var v: dynamic;

var dist = cpp_array(100000, 2);

var g: dynamic;

func dfs(v: dynamic, prev: dynamic, d: dynamic, f: dynamic)
{
  dist[f][v] = d;
  for (var e in g[v])
  {
    if ((e != prev))
    {
      dfs(e, v, (d + 1), f);
    }
  }
}

func main()
{
  read(n, u, v);
  u -= 1;
  v -= 1;
  g.resize(n);
  {
    var i = 1;
    while ((i < n))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      a -= 1;
      b -= 1;
      g[a].push_back(b);
      g[b].push_back(a);
      i += 1;
    }
  }
  dfs(u, -1, 0, 0);
  dfs(v, -1, 0, 1);
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((dist[0][i] < dist[1][i]))
      {
        ans = max(ans, (dist[1][i] - 1));
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
