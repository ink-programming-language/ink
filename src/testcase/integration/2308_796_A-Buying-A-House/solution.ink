// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  var a: dynamic;
  var jwb = 0;
  var jwbn = 0;
  read(n, m, k);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a);
      if (((a <= k) && (a > 0)))
      {
        if ((jwbn == 0))
        {
          jwbn = i;
          jwb = a;
        } else if ((abs((m - i)) < abs((m - jwbn))))
        {
          jwbn = i;
          jwb = a;
        }
      }
      i += 1;
    }
  }
  write((abs((jwbn - m)) * 10), "\n");
  return 0;
}
