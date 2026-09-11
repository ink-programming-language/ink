// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var sum: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    sum = 0;
    var p: dynamic = ((n / 2) + 1);
    {
      var i: dynamic = p;
      while ((p <= n))
      {
        var x: dynamic = ((n - i) + 1);
        var s: dynamic = 1;
        {
          var k: dynamic = 0;
          while ((k < (p - x)))
          {
            s *= 2;
            k += 1;
          }
        }
        sum += s;
        p += 1;
      }
    }
    write(sum, "\n");
  }
  return 0;
}
