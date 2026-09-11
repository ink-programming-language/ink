// Translated from solution.cpp.

var vis: dynamic = cpp_array(200010);

func dfs(arr: dynamic, i: dynamic, temp: dynamic) -> dynamic
{
  vis[i] = true;
  for (var u: dynamic in arr[i])
  {
    if (cpp_binary((!vis[u]), "and", (u != temp)))
    {
      dfs(arr, u, temp);
    }
  }
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var u: dynamic = cpp_uninitialized();
    var v: dynamic = cpp_uninitialized();
    read(n, m, a, b);
    var arr: dynamic = cpp_array((n + 1));
    {
      var i: dynamic = 0;
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
    var us1: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 1;
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
    var us2: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 1;
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
    var ans1: dynamic = 0;
    var ans2: dynamic = 0;
    for (var val: dynamic in us1)
    {
      if ((us2.find(val) == us2.end()))
      {
        ans1 += 1;
      }
    }
    for (var val: dynamic in us2)
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
