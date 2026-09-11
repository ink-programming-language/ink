// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  n -= 43500;
  puts( ((n > 0)) ? ( ((n > 2000)) ? "3" : "2") : "1");
  return 0;
}
