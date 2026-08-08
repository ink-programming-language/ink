// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var x: dynamic;
  var y: dynamic;
  var k: dynamic;
  var t: dynamic;
  var i: dynamic;
  var r: dynamic;
  read(t);
  {
    i = 0;
    while ((i < t))
    {
      read(x, y, n);
      r = (((((n / x)) * x)) + y);
      if ((r > n))
      {
        r -= x;
      }
      write(r, "\n");
      i += 1;
    }
  }
}
