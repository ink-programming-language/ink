// Translated from solution.cpp.

var maxn = 1000100;

var result: dynamic;

var n: dynamic;

var m: dynamic;

var deg = cpp_array((maxn + 1));

func main()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var x: dynamic;
  var y: dynamic;
  scanf("%I64d%I64d", (&n), (&m));
  {
    i = 1;
    while ((i <= n))
    {
      deg[i] = 0;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= m))
    {
      scanf("%I64d%I64d", (&x), (&y));
      deg[x] += 1;
      deg[y] += 1;
      i += 1;
    }
  }
  result = 0;
  {
    i = 1;
    while ((i <= n))
    {
      result += (deg[i] * (((n - 1) - deg[i])));
      i += 1;
    }
  }
  result = ((((n * ((n - 1))) * ((n - 2))) / 6) - (result / 2));
  printf("%I64d\n", result);
  return 0;
}
