// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  while ((a != b))
  {
    if ((a > b))
    {
      a = (a - b);
    } else
    {
      b = (b - a);
    }
  }
  return a;
}

func main()
{
  var y: dynamic;
  var k: dynamic;
  var n: dynamic;
  read(y, k, n);
  var f = 0;
  var d = (y / k);
  var e = (n / k);
  {
    var i = (d + 1);
    while ((i <= e))
    {
      write(((k * i) - y), " ");
      f = 1;
      i += 1;
    }
  }
  if ((f == 0))
  {
    write(-1, "\n");
  }
  return 0;
}
