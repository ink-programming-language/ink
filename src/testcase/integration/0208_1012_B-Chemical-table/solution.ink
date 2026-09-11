// Translated from solution.cpp.

var N: dynamic = (4e5 + 10);

var fa: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

func find(x: dynamic) -> dynamic
{
  return  ((x == fa[x])) ? x : cpp_assign(fa[x], "=", find(fa[x]));
}

func solve() -> dynamic
{
  scanf("%d%d%d", (&n), (&m), (&q));
  {
    var i: dynamic = 1;
    while ((i <= (n + m)))
    {
      fa[i] = i;
      i += 1;
    }
  }
  var res: dynamic = ((n + m) - 1);
  {
    var i: dynamic = 1;
    while ((i <= q))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      scanf("%d%d", (&x), (&y));
      var fx: dynamic = find(x);
      var fy: dynamic = find((y + n));
      if ((fx != fy))
      {
        fa[fx] = fy;
        res -= 1;
      }
      i += 1;
    }
  }
  printf("%d\n", res);
}

func main() -> dynamic
{
  solve();
  return 0;
}
