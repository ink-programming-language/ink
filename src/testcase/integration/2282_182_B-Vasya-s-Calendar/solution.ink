// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var d: dynamic;
  read(d, n);
  var ilosc = 0;
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      var p: dynamic;
      read(p);
      ilosc += ((d - p));
      i += 1;
    }
  }
  read(d);
  write(ilosc, "\n");
  return 0;
}
