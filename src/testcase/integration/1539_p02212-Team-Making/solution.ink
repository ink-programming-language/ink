// Translated from solution.cpp.

func main() -> dynamic
{
  var A: dynamic = cpp_uninitialized();
  var B: dynamic = cpp_uninitialized();
  var C: dynamic = cpp_uninitialized();
  var D: dynamic = cpp_uninitialized();
  read(A, B, C, D);
  write(min(min(abs((((A + B) - C) - D)), abs((((A + C) - B) - D))), abs((((A + D) - B) - C))), "\n");
}
