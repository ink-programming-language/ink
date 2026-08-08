// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  {
    var x = 0;
    while (((x * 4) <= n))
    {
      var r = (n - ((4 * x)));
      if (((r % 7) == 0))
      {
        {
          var i = 1;
          while ((i <= x))
          {
            write(4);
            i += 1;
          }
        }
        {
          var j = 1;
          while ((j <= ((r / 7))))
          {
            write(7);
            j += 1;
          }
        }
        write("\n");
        return 0;
      }
      x += 1;
    }
  }
  write(-1, "\n");
  return 0;
}
