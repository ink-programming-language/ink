// Translated from solution.cpp.

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = 0;

func main() -> dynamic
{
  scanf("%d%d", (&a), (&b));
  while (b)
  {
    c = ((c * 10) + (b % 10));
    b /= 10;
  }
  printf("%d\n", (a + c));
}
