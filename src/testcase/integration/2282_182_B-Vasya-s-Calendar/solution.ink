// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(d, n);
  var ilosc: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var p: dynamic = cpp_uninitialized();
      read(p);
      ilosc += ((d - p));
      i += 1;
    }
  }
  read(d);
  write(ilosc, "\n");
  return 0;
}
