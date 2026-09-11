// Translated from solution.cpp.

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var cc: dynamic = [0];
  var m: dynamic = 0;
  var s: dynamic = 0;
  {
    i = 0;
    while ((i < 5))
    {
      read(n);
      s += n;
      cc[n] += 1;
      if (((cc[n] == 3) || (cc[n] == 2)))
      {
        m = max(m, (cc[n] * n));
      }
      i += 1;
    }
  }
  write((s - m), "\n");
}
