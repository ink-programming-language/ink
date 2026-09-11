// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  read(n, m);
  while (cpp_update(m, "--"))
  {
    read(x);
    n -= x;
  }
  if ((n < 0))
  {
    write("-1", "\n");
  } else
  {
    write(n, "\n");
  }
}
