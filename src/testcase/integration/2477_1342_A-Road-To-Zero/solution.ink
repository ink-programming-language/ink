// Translated from solution.cpp.

var N: dynamic = 200005;

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  scanf("%d", (&t));
  while (cpp_update(t, "--"))
  {
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    scanf("%d%d%d%d", (&x), (&y), (&a), (&b));
    if (((x < 0) && (y > 0)))
    {
      printf("%lld\n", ((((1 * abs(x)) + abs(y))) * a));
    } else if (((x > 0) && (y < 0)))
    {
      printf("%lld\n", ((((1 * abs(x)) + abs(y))) * a));
    } else
    {
      x = abs(x);
      y = abs(y);
      var mx: dynamic = max(x, y);
      var mn: dynamic = min(x, y);
      printf("%lld\n", ((((mx - mn)) * a) + (mn * min(b, (2 * a)))));
    }
  }
}
