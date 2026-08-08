// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var ma: dynamic;
  read(n, ma);
  var ans = 0;
  {
    var i = 1;
    while ((i < n))
    {
      var x: dynamic;
      read(x);
      if ((ma > x))
      {
        ans += (ma - x);
      }
      ma = max(ma, x);
      i += 1;
    }
  }
  write(ans);
  return 0;
}
