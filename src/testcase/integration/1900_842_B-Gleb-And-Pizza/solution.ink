// Translated from solution.cpp.

func main()
{
  var r: dynamic;
  var d: dynamic;
  read(r, d);
  var n: dynamic;
  read(n);
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
      var y: dynamic;
      var r1: dynamic;
      read(x, y, r1);
      var c = sqrt(((x * x) + (y * y)));
      if ((((r - c) >= r1) && ((r - c) <= (d - r1))))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
