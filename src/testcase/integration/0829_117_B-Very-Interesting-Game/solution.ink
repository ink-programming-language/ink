// Translated from solution.cpp.

var MAX: dynamic = ((1000 * 1000) * 1000);

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  read(a, b, m);
  if (((m <= (b + 1)) || ((MAX % m) == 0)))
  {
    write(2);
    return 0;
  }
  {
    i = 1;
    while ((i <= min((m - 1), a)))
    {
      var k: dynamic = ((MAX * i) % m);
      if (((0 < k) && (k < (m - b))))
      {
        printf("1\n%09I64d", i);
        return 0;
      }
      i += 1;
    }
  }
  write(2);
  return 0;
}
