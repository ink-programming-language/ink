// Translated from solution.cpp.

var vis = cpp_array(200010);

func dfs(arr: dynamic, i: dynamic, temp: dynamic)
{
  vis[i] = true;
  for (var u in arr[i])
  {
    if (cpp_binary((!vis[u]), "and", (u != temp)))
    {
      dfs(arr, u, temp);
    }
  }
}

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var m: dynamic;
    var a: dynamic;
    var b: dynamic;
    var u: dynamic;
    var v: dynamic;
    read(n, m, a, b);
    var arr = cpp_array((n + 1));
    {
      var i = 0;
      while ((i < m))
      {
        read(u, v);
        arr[u].push_back(v);
        arr[v].push_back(u);
        i += 1;
      }
    }
    memset(vis, false, cpp_sizeof((vis)));
    dfs(arr, a, b);
    var us1: dynamic;
    {
      var i = 1;
      while ((i <= n))
      {
        if (vis[i])
        {
          us1.insert(i);
        }
        i += 1;
      }
    }
    us1.erase(a);
    memset(vis, false, cpp_sizeof((vis)));
    dfs(arr, b, a);
    var us2: dynamic;
    {
      var i = 1;
      while ((i <= n))
      {
        if (vis[i])
        {
          us2.insert(i);
        }
        i += 1;
      }
    }
    us2.erase(b);
    var ans1 = 0;
    var ans2 = 0;
    for (var val in us1)
    {
      if ((us2.find(val) == us2.end()))
      {
        ans1 += 1;
      }
    }
    for (var val in us2)
    {
      if ((us1.find(val) == us1.end()))
      {
        ans2 += 1;
      }
    }
    write((ans1 * ans2), "\n");
  }
  return 0;
}
