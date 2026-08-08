// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
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
