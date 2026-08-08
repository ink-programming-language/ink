// Translated from solution.cpp.

var mod = (((1000 * 1000) * 1000) + 7);

var INF = (1e9 + 100);

var LINF = (1e18 + 100);

func main(argument_0: dynamic)
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var a: dynamic;
  var b: dynamic;
  read(a, b);
  var x = 0;
  var ans = 0;
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
