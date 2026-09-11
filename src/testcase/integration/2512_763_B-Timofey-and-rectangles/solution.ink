// Translated from solution.cpp.

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

func init() -> dynamic
{
  scanf("%d", (&n));
  puts("YES");
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d%d%d%d", (&x), (&y), (&a), (&b));
      printf("%d\n", (((2 * abs((x % 2))) + abs((y % 2))) + 1));
      i += 1;
    }
  }
}

func main() -> dynamic
{
  init();
  return 0;
}
