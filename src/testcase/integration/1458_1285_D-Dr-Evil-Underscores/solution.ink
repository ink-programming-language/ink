// Translated from solution.cpp.

var N: dynamic = (1e5 + 5);

var cnt: dynamic = 1;

var T: dynamic = cpp_array(2, (N * 31));

var n: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array((N * 31));

func insert(x: dynamic) -> dynamic
{
  var u: dynamic = 0;
  {
    var i: dynamic = 30;
    while ((i >= 0))
    {
      var v: dynamic = (((x >> i)) & 1);
      if ((!T[u][v]))
      {
        memset(T[cnt], 0, cpp_sizeof((T[cnt])));
        a[cnt] = 0;
        T[u][v] = cpp_update(cnt, "++");
      }
      u = T[u][v];
      i -= 1;
    }
  }
  a[u] = x;
}

func dfs(p: dynamic, k: dynamic) -> dynamic
{
  if (((!T[p][0]) && (!T[p][1])))
  {
    return 0;
  }
  if ((!T[p][0]))
  {
    return dfs(T[p][1], (k - 1));
  }
  if ((!T[p][1]))
  {
    return dfs(T[p][0], (k - 1));
  }
  return (((1 << k)) + min(dfs(T[p][0], (k - 1)), dfs(T[p][1], (k - 1))));
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lld", (&t));
      insert(t);
      i += 1;
    }
  }
  printf("%lld\n", dfs(0, 30));
  return 0;
}
