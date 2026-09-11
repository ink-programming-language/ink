// Translated from solution.cpp.

func main(argument_0: dynamic) -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var x: dynamic = 0;
  scanf("%d %d %d", (&a), (&b), (&c));
  {
    x = a;
    while ((x <= b))
    {
      if (((c % x) == 0))
      {
        d += 1;
      }
      x += 1;
    }
  }
  printf("%d\n", d);
  return 0;
}
