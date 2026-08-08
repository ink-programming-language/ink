// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  var n: dynamic;
  var h: dynamic;
  var f = cpp_construct(6, 0);
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
