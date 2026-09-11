// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    read(n, m);
    if (((n == 1) || (m == 1)))
    {
      write("YES", "\n");
    } else if (((n == 2) && (m == 2)))
    {
      write("YES", "\n");
    } else
    {
      write("NO", "\n");
    }
  }
  return 0;
}
