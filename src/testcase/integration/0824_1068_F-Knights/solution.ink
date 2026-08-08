// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var y = 0;
  var k = 0;
  while ((y != n))
  {
    if (((k % 2) == 0))
    {
      write(k, "  0", "\n");
      y += 1;
    } else
    {
      if (((n - y) == 1))
      {
        write(k, "  0", "\n");
        y += 1;
      } else if (((k % 2) == 1))
      {
        write(k, "  0", "\n");
        write(k, "  3", "\n");
        y += 2;
      }
    }
    k += 1;
  }
}
