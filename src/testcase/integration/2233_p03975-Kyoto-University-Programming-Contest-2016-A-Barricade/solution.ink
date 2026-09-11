// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var t: dynamic = 0;
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  read(n, m, k);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(l);
      if (((l < m) || (l >= k)))
      {
        t += 1;
      }
      i += 1;
    }
  }
  write(t, "\n");
  return 0;
}
