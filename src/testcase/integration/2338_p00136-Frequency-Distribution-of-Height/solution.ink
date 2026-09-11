// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_construct(6, 0);
  read(n);
  {
    while (cpp_update(n, "--"))
    {
      read(t);
      if ((t < 165.0))
      {
        h = 0;
      } else
      {
        h = (((cpp_cast(t) - 160)) / 5);
      }
      if ((h > 5))
      {
        h = 5;
      }
      f[h] += 1;
    }
  }
  {
    n = 0;
    while ((n < 6))
    {
      write((n + 1), ":");
      while (cpp_update(f[n], "--"))
      {
        write("*");
      }
      write("\n");
      n += 1;
    }
  }
  return 0;
}
