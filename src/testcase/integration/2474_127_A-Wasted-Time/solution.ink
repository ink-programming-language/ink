// Translated from solution.cpp.

func absolute(a: dynamic, b: dynamic)
{
  write(setprecision(20));
  return sqrt(((a * a) + (b * b)));
}

func main()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var a: dynamic;
  var b: dynamic;
  var pa = 0;
  var pb = 0;
  var len = 0;
  {
    var i = 0;
    while ((i < n))
    {
      read(a, b);
      if ((i != 0))
      {
        len += absolute(abs((a - pa)), abs((b - pb)));
      }
      pa = a;
      pb = b;
      i += 1;
    }
  }
  write((((len * k)) / 50));
}
