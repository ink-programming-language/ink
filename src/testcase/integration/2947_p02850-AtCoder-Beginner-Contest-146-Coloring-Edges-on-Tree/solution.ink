// Translated from solution.cpp.

var IOS: dynamic = cpp_expression("#include <bits/stdc++.h>");

var endl: dynamic = cpp_expression("#inc");

var int_cpp: dynamic = dynamic;

var N: dynamic = (1e5 + 5);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var deg: dynamic = cpp_array(N);

var ans: dynamic = cpp_array(N);

var g: dynamic = cpp_array(N);

func dfs(u: dynamic, par: dynamic, c: dynamic) -> dynamic
{
  for (var it: dynamic in g[u])
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

func main() -> dynamic
{
  IOS;
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= (n - 1)))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      deg[u] += 1;
      deg[v] += 1;
      g[u].push_back([v, i]);
      g[v].push_back([u, i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      k = max(k, deg[i]);
      i += 1;
    }
  }
  dfs(1, 1, 0);
  write(k, "\n");
  {
    var i: dynamic = 1;
    while ((i <= (n - 1)))
    {
      write((ans[i] + 1), "\n");
      i += 1;
    }
  }
  return 0;
}
