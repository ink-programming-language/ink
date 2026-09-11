// Translated from solution.cpp.

var ans: dynamic = cpp_uninitialized();

func dfs(to: dynamic, v: dynamic, p: dynamic = -1) -> dynamic
{
  for (var u: dynamic in to[v])
  {
    if ((u == p))
    {
      continue;
    }
    ans[u] += ans[v];
    dfs(to, u, v);
  }
}

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(n, q);
  var to: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
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
    var i: dynamic = 0;
    while ((i < q))
    {
      var p: dynamic = cpp_uninitialized();
      var x: dynamic = cpp_uninitialized();
      read(p, x);
      p -= 1;
      ans[p] += x;
      i += 1;
    }
  }
  dfs(to, 0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(ans[i], "\n");
      i += 1;
    }
  }
  return 0;
}
