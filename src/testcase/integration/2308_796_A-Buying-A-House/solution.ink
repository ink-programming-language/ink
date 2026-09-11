// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var jwb: dynamic = 0;
  var jwbn: dynamic = 0;
  read(n, m, k);
  {
    var i: dynamic = 1;
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
