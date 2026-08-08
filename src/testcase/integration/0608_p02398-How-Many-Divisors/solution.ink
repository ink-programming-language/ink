// Translated from solution.cpp.

func main(argument_0: dynamic)
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  var x = 0;
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
