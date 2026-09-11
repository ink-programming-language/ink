// Translated from solution.cpp.

func main() -> dynamic
{
  var A: dynamic = cpp_uninitialized();
  var B: dynamic = cpp_uninitialized();
  var C: dynamic = cpp_uninitialized();
  read(A, B, C);
  puts( (((A + B) < C)) ? "No" : "Yes");
}
