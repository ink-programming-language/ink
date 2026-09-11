// Translated from solution.cpp.

func gcd(x: dynamic, y: dynamic) -> dynamic
{
  while (((cpp_assign(x, "%=", y)) && (cpp_assign(y, "%=", x))))
  {
  }
  return (x ^ y);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var T: dynamic = cpp_uninitialized();
  read(T);
  while (cpp_update(T, "--"))
  {
    var A: dynamic = cpp_uninitialized();
    var B: dynamic = cpp_uninitialized();
    var C: dynamic = cpp_uninitialized();
    var D: dynamic = cpp_uninitialized();
    read(A, B, C, D);
    if (((A < B) || (D < B)))
    {
      write("No\n");
      continue;
    }
    var g: dynamic = gcd(B, D);
    A = (A - (((C - B) + 1)));
    var d: dynamic = ((((A % g) + g)) % g);
    A = (((C - B) + 1) + d);
    if ((A < 0))
    {
      write("No\n");
    } else
    {
      write("Yes\n");
    }
  }
}
