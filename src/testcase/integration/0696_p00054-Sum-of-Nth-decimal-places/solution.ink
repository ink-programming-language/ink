// Translated from solution.cpp.

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  while ((((cin >> a) >> b) >> n))
  {
    var ans: dynamic = 0;
    a = (10 * ((a % b)));
    while (cpp_update(n, "--"))
    {
      ans += (a / b);
      a = (10 * ((a % b)));
    }
    write(ans, "\n");
  }
  return 0;
}
