// Translated from solution.cpp.

var oo: dynamic = 1e9;

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  read(a, b, c);
  var ans: dynamic = (2 * ((a + b)));
  ans = min(ans, (2 * ((a + c))));
  ans = min(ans, (2 * ((b + c))));
  ans = min(ans, ((a + b) + c));
  write(ans, cpp_char("\n"));
  return 0;
}
