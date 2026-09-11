// Translated from solution.cpp.

func main() -> dynamic
{
  var x1: dynamic = cpp_uninitialized();
  var y1: dynamic = cpp_uninitialized();
  var x2: dynamic = cpp_uninitialized();
  var y2: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  read(x1, y1, x2, y2);
  ans = sqrt(((((x2 - x1)) * ((x2 - x1))) + (((y2 - y1)) * ((y2 - y1)))));
  write(fixed, ans, "\n");
}
