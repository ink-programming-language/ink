// Translated from solution.cpp.

func main()
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  read(a, b, c, d);
  {
    var i = 0;
    while ((i <= 1e6))
    {
      var x = ((((i * c)) + d) - b);
      if ((x < 0))
      {
        i += 1;
        continue;
      }
      if (((x % a) == 0))
      {
        write((x + b));
        return 0;
      }
      i += 1;
    }
  }
  write("-1");
}
