// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic) -> dynamic
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

func main() -> dynamic
{
  var y: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  read(y, k, n);
  var f: dynamic = 0;
  var d: dynamic = (y / k);
  var e: dynamic = (n / k);
  {
    var i: dynamic = (d + 1);
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
