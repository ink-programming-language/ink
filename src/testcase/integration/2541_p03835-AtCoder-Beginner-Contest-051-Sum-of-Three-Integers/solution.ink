// Translated from solution.cpp.

func main() -> dynamic
{
  var ans: dynamic = 0;
  var k: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  read(k, s);
  {
    x = 0;
    while ((x <= k))
    {
      {
        y = 0;
        while ((y <= k))
        {
          if (((((s - x) - y) >= 0) && (((s - x) - y) <= k)))
          {
            ans += 1;
          }
          y += 1;
        }
      }
      x += 1;
    }
  }
  write(ans, "\n");
}
