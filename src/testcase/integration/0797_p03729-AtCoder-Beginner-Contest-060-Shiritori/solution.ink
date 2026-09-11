// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  read(a, b, c);
  write(( ((((a[(a.size() - 1)] == b[0]) && (b[(b.size() - 1)] == c[0])))) ? "YES" : "NO"), "\n");
  return (0);
}
