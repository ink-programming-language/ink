// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    if (((n % 2) == 0))
    {
      if ((n == 2))
      {
        write(2, "\n");
      } else
      {
        write(0, "\n");
      }
    } else
    {
      write(1, "\n");
    }
  }
}
