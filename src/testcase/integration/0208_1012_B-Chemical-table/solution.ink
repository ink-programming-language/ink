// Translated from solution.cpp.

var N = (4e5 + 10);

var fa = cpp_array(N);

var n: dynamic;

var m: dynamic;

var q: dynamic;

func find(x: dynamic)
{
  return if ((x == fa[x])) x else cpp_assign(fa[x], "=", find(fa[x]));
}

func solve()
{
  scanf("%d%d%d", (&n), (&m), (&q));
  {
    var i = 1;
    while ((i <= (n + m)))
    {
      fa[i] = i;
      i += 1;
    }
  }
  var res = ((n + m) - 1);
  {
    var i = 1;
    while ((i <= q))
    {
      var x: dynamic;
      var y: dynamic;
      scanf("%d%d", (&x), (&y));
      var fx = find(x);
      var fy = find((y + n));
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

func main()
{
  solve();
  return 0;
}
