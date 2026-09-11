// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    read(a);
    var ans: dynamic = 0;
    while ((a > 0))
    {
      ans += (a % 2);
      a /= 2;
    }
    ans = pow(2, ans);
    write(ans, "\n");
  }
  return 0;
}
