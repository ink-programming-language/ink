// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d%d", (&n), (&k));
  k = ((((n - k)) / 2) + 1);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      putchar((cpp_char("0") + (!((i % k)))));
      i += 1;
    }
  }
  return 0;
}
