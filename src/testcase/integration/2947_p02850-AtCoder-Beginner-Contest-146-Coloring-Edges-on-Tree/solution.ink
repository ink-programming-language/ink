// Translated from solution.cpp.

var IOS = cpp_expression("#include <bits/stdc++.h>");

var endl = cpp_expression("#inc");

var int_cpp = dynamic;

var N = (1e5 + 5);

var n: dynamic;

var k: dynamic;

var deg = cpp_array(N);

var ans = cpp_array(N);

var g = cpp_array(N);

func dfs(u: dynamic, par: dynamic, c: dynamic)
{
  for (var it in g[u])
  {
    if ((it.first == par))
    {
      continue;
    }
    c = (((c + 1)) % k);
    ans[it.second] = c;
    dfs(it.first, u, c);
  }
}

func main()
{
  IOS;
  read(n);
  {
    var i = 1;
    while ((i <= (n - 1)))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      deg[u] += 1;
      deg[v] += 1;
      g[u].push_back([v, i]);
      g[v].push_back([u, i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      k = max(k, deg[i]);
      i += 1;
    }
  }
  dfs(1, 1, 0);
  write(k, "\n");
  {
    var i = 1;
    while ((i <= (n - 1)))
    {
      write((ans[i] + 1), "\n");
      i += 1;
    }
  }
  return 0;
}
