// Translated from solution.cpp.

var N = 200005;

var n: dynamic;

var a = cpp_array(N);

func main(argc: dynamic, argv: dynamic)
{
  var t: dynamic;
  scanf("%d", (&t));
  while (cpp_update(t, "--"))
  {
    var x: dynamic;
    var y: dynamic;
    var a: dynamic;
    var b: dynamic;
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
      var mx = max(x, y);
      var mn = min(x, y);
      printf("%lld\n", ((((mx - mn)) * a) + (mn * min(b, (2 * a)))));
    }
  }
}
