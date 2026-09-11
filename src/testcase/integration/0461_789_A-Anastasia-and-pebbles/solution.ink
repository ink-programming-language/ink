// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var icount: dynamic = cpp_uninitialized();
  while ((scanf("%d%d", (&n), (&k)) != EOF))
  {
    icount = 0;
    while (((cpp_update(n, "--")) > 0))
    {
      scanf("%d", (&t));
      icount += ((((t + k) - 1)) / k);
    }
    printf("%d\n", (((icount + 1)) / 2));
  }
  return 0;
}
