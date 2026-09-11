// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var c: dynamic = 0;
  scanf("%llu", (&n));
  if ((n < 2520))
  {
    printf("0\n");
  } else
  {
    printf("%llu\n", (n / 2520));
  }
  return 0;
}
