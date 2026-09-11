// Translated from solution.cpp.

var maxn: dynamic = 1000100;

var result: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var deg: dynamic = cpp_array((maxn + 1));

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
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
