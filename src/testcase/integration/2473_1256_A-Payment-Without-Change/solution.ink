// Translated from solution.cpp.

func main() -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  read(q);
  while (cpp_update(q, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var N: dynamic = cpp_uninitialized();
    var s: dynamic = cpp_uninitialized();
    read(a, b, N, s);
    if (((s > ((N * a) + b)) || (b < (s % N))))
    {
      write("NO", "\n");
    } else
    {
      write("YES", "\n");
    }
  }
  return 0;
}
