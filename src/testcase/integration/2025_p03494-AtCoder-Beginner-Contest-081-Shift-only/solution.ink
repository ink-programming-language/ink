// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var ans: dynamic = 1000000;
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a);
      k = 0;
      while (((a % 2) == 0))
      {
        k += 1;
        a /= 2;
      }
      ans = min(ans, k);
      i += 1;
    }
  }
  write(ans, "\n");
}
