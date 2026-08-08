// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  read(N);
  if ((N & 1))
  {
    write(-1, "\n");
    return 0;
  }
  {
    var i = 0;
    while ((i < N))
    {
      write("\n");
      {
        var j = 0;
        while ((j < N))
        {
          write("\n");
          {
            var k = 0;
            while ((k < N))
            {
              var n = min(min(j, k), min(((N - 1) - j), ((N - 1) - k)));
              if ((((i % 2)) ^ ((n % 2))))
              {
                write("b");
              } else
              {
                write("w");
              }
              k += 1;
            }
          }
          j += 1;
          write("\n");
        }
      }
      i += 1;
    }
  }
  return 0;
}
