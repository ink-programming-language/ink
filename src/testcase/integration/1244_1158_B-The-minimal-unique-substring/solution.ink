// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

func main()
{
  scanf("%d%d", (&n), (&k));
  k = ((((n - k)) / 2) + 1);
  {
    var i = 1;
    while ((i <= n))
    {
      putchar((cpp_char("0") + (!((i % k)))));
      i += 1;
    }
  }
  return 0;
}
