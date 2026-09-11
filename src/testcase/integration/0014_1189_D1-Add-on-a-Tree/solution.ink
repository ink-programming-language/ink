// Translated from solution.cpp.

func pr_init() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
}

var tree: dynamic = cpp_uninitialized();

var deg: dynamic = cpp_uninitialized();

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  tree.assign((n + 1), vector());
  deg.assign((n + 1), 0);
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      deg[u] += 1;
      deg[v] += 1;
      tree[u].emplace_back(v);
      tree[v].emplace_back(u);
      i += 1;
    }
  }
  if ((n == 2))
  {
    write("YES\n");
    return;
  } else if ((n == 3))
  {
    write("NO\n");
    return;
  }
  var is: dynamic = true;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (((deg[i] != 1) && (deg[i] < 3)))
      {
        is = false;
      }
      i += 1;
    }
  }
  if (is)
  {
    write("YES\n");
  } else
  {
    write("NO\n");
  }
}

func main() -> dynamic
{
  solve();
  return 0;
}
