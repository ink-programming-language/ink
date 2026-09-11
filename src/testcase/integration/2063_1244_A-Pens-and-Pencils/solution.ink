// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var c: dynamic = cpp_uninitialized();
    var d: dynamic = cpp_uninitialized();
    var k: dynamic = cpp_uninitialized();
    var result: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    read(a, b, c, d, k);
    if (((a % c) == 0))
    {
      x = (a / c);
    } else
    {
      x = ((a / c) + 1);
    }
    if (((b % d) == 0))
    {
      y = (b / d);
    } else
    {
      y = ((b / d) + 1);
    }
    if (((x + y) > k))
    {
      write("-1", "\n");
    } else
    {
      write((k - y), "\n");
      write(y, "\n");
    }
  }
}
