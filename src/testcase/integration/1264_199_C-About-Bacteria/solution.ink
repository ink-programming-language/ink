// Translated from solution.cpp.

func main() -> dynamic
{
  var k: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  while (((((cin >> k) >> b) >> n) >> t))
  {
    var s: dynamic = 1;
    var cas: dynamic = 0;
    while (((s <= t) && (cas < n)))
    {
      s = ((s * k) + b);
      cas += 1;
    }
    if (((cas == n) && (s <= t)))
    {
      write(0, "\n");
    } else
    {
      write(((n - cas) + 1), "\n");
    }
  }
  return 0;
}
