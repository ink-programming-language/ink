// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var d: dynamic;
  var ans = 0;
  read(n, d);
  var x: dynamic;
  var y: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      read(x, y);
      if ((((x * x) + (y * y)) <= (d * d)))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  write(ans);
  return 0;
}
