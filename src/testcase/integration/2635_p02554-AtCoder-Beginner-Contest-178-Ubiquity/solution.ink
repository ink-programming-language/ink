// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = 1;
  var b: dynamic = 1;
  var c: dynamic = 1;
  var ans: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      a = (((a * 10)) % 1000000007);
      b = (((b * 9)) % 1000000007);
      c = (((c * 8)) % 1000000007);
      i += 1;
    }
  }
  ans = ((((a - (2 * b)) + c)) % 1000000007);
  ans = (((ans + 1000000007)) % 1000000007);
  write(ans);
  return 0;
}
