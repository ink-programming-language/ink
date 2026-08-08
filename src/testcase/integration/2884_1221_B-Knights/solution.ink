// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
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
