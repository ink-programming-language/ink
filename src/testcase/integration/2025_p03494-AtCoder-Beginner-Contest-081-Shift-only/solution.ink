// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var a: dynamic;
  var k: dynamic;
  var ans = 1000000;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a);
      k = 0;
      while (((a % 2) == 0))
      {
        k += 1;
        a /= 2;
      }
      ans = min(ans, k);
      i += 1;
    }
  }
  write(ans, "\n");
}
