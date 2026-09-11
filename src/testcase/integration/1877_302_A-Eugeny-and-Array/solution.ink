// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&m));
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var one: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&a));
      if ((a == 1))
      {
        one += 1;
      }
      i += 1;
    }
  }
  var pone: dynamic = (n - one);
  while (cpp_update(m, "--"))
  {
    scanf("%d%d", (&a), (&b));
    var tmp: dynamic = ((b - a) + 1);
    if ((tmp & 1))
    {
      puts("0");
    } else
    {
      if ((((tmp / 2) <= one) && ((tmp / 2) <= pone)))
      {
        puts("1");
      } else
      {
        puts("0");
      }
    }
  }
  return 0;
}
