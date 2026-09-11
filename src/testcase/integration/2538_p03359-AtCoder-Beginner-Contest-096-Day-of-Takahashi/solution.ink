// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(a, b);
  var ans: dynamic =  ((a > b)) ? (a - 1) : a;
  write(ans, "\n");
}
