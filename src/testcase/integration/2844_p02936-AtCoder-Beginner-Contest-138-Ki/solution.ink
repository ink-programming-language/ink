// Translated from solution.cpp.

var ans: dynamic;

func dfs(to: dynamic, v: dynamic, p: dynamic = -1)
{
  for (var u in to[v])
  {
    if ((u == p))
    {
      continue;
    }
    ans[u] += ans[v];
    dfs(to, u, v);
  }
}

func main(argc: dynamic, argv: dynamic)
{
  var n: dynamic;
  var q: dynamic;
  read(n, q);
  var to = cpp_array(n);
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      a -= 1;
      b -= 1;
      to[a].push_back(b);
      to[b].push_back(a);
      i += 1;
    }
  }
  ans.resize(n);
  {
    var i = 0;
    while ((i < q))
    {
      var p: dynamic;
      var x: dynamic;
      read(p, x);
      p -= 1;
      ans[p] += x;
      i += 1;
    }
  }
  dfs(to, 0);
  {
    var i = 0;
    while ((i < n))
    {
      write(ans[i], "\n");
      i += 1;
    }
  }
  return 0;
}
