// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(a, b);
  var ans: dynamic = 0;
  var t: dynamic = 1;
  while ((a || b))
  {
    var x: dynamic = (a % 3);
    var y: dynamic = (b % 3);
    var z: dynamic = ((((y - x) + 3)) % 3);
    ans += (z * t);
    t *= 3;
    a /= 3;
    b /= 3;
  }
  write(ans, "\n");
}
