// Translated from solution.cpp.

func pr_init()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
}

var tree: dynamic;

var deg: dynamic;

func solve()
{
  var n: dynamic;
  read(n);
  tree.assign((n + 1), vector());
  deg.assign((n + 1), 0);
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      var u: dynamic;
      var v: dynamic;
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
  var is = true;
  {
    var i = 1;
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

func main()
{
  solve();
  return 0;
}
