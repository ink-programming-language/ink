// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  read(a, b, c);
  write(( ((((((a == b)) + ((b == c))) + ((c == a))) == 1)) ? "Yes" : "No"), "\n");
}
