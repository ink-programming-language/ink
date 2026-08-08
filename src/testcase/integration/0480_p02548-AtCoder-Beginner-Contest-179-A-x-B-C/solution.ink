// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var count = 0;
  read(n);
  {
    var a = 1;
    while ((a <= (n - 1)))
    {
      {
        var b = 1;
        while ((b <= (((n - 1)) / a)))
        {
          count += 1;
          b += 1;
        }
      }
      a += 1;
    }
  }
  write(count);
}
