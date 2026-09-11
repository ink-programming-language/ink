// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var count: dynamic = 0;
  read(n);
  {
    var a: dynamic = 1;
    while ((a <= (n - 1)))
    {
      {
        var b: dynamic = 1;
        while ((b <= (((n - 1)) / a)))
        {
          count += 1;
          b += 1;
        }
      }
      a += 1;
    }
  }
  write(count);
}
