// Translated from solution.cpp.

func main() -> dynamic
{
  var h: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  read(h, w, n);
  write(((((n + max(h, w)) - 1)) / max(h, w)), "\n");
}
