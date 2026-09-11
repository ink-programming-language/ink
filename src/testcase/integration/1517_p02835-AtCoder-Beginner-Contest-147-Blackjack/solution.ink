// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  scanf("%d%d%d", (&a), (&b), (&c));
  s = ((a + b) + c);
  if ((s >= 22))
  {
    printf("bust\n");
  } else
  {
    printf("win\n");
  }
}
