// Translated from solution.cpp.

func main() -> dynamic
{
  var i: dynamic = 0;
  var k: dynamic = 0;
  var m: dynamic = cpp_uninitialized();
  var s1: dynamic = 0;
  var j: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var r: dynamic = 0;
  var l: dynamic = 0;
  var l1: dynamic = 0;
  var c: dynamic = 0;
  var t: dynamic = cpp_uninitialized();
  var d: dynamic = 0;
  var na: dynamic = cpp_uninitialized();
  var nb: dynamic = cpp_uninitialized();
  read(n, m, k);
  {
    i = 1;
    while ((i <= n))
    {
      if (((m >= (2 * i)) && (k >= (4 * i))))
      {
        d = i;
      }
      i += 1;
    }
  }
  write(((d) * 7));
  return 0;
}
