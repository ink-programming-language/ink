// Translated from solution.cpp.

var mod: dynamic = (((1000 * 1000) * 1000) + 7);

var INF: dynamic = (1e9 + 100);

var LINF: dynamic = (1e18 + 100);

func main(argument_0: dynamic) -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(a, b);
  var x: dynamic = 0;
  var ans: dynamic = 0;
  while (a)
  {
    ans += a;
    x += a;
    a = (x / b);
    x %= b;
  }
  write(ans, cpp_char("\n"));
  return 0;
}
