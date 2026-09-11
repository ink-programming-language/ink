// Translated from solution.cpp.

func main() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  scanf("%d %d", (&x), (&t));
  printf("%d",  (((x - t) < 0)) ? 0 : (x - t));
  return 0;
}
