// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var m: dynamic;
  read(a, b);
  m = b;
  {
    var i = 1;
    while ((i <= (a - 1)))
    {
      b *= ((m - 1));
      i += 1;
    }
  }
  write(b);
  return 0;
}
