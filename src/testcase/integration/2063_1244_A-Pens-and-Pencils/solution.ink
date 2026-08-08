// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var a: dynamic;
    var b: dynamic;
    var c: dynamic;
    var d: dynamic;
    var k: dynamic;
    var result: dynamic;
    var x: dynamic;
    var y: dynamic;
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
