// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          if (((i % 2) == 1))
          {
            if (((j % 2) == 1))
            {
              write("W");
            } else
            {
              write("B");
            }
          } else
          {
            if (((j % 2) == 1))
            {
              write("B");
            } else
            {
              write("W");
            }
          }
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
  return 0;
}
