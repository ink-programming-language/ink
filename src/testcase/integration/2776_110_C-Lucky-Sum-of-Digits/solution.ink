// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var x: dynamic = 0;
    while (((x * 4) <= n))
    {
      var r: dynamic = (n - ((4 * x)));
      if (((r % 7) == 0))
      {
        {
          var i: dynamic = 1;
          while ((i <= x))
          {
            write(4);
            i += 1;
          }
        }
        {
          var j: dynamic = 1;
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
