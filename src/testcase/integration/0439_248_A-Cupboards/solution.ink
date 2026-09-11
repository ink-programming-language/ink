// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var cell: dynamic = cpp_array(1001);
  read(n);
  k = 0;
  p = 0;
  {
    (i) = (0);
    while (((i) < cpp_cast((n))))
    {
      read(a, b);
      if ((a == 0))
      {
        k += 1;
      }
      if ((b == 0))
      {
        p += 1;
      }
      (i) += 1;
    }
  }
  var sum: dynamic = cpp_uninitialized();
  sum = 0;
  sum += min(k, (n - k));
  sum += min(p, (n - p));
  write(sum, "\n");
  return 0;
}
