// Translated from solution.cpp.

var oo = 1e9;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  read(a, b, c);
  var ans = (2 * ((a + b)));
  ans = min(ans, (2 * ((a + c))));
  ans = min(ans, (2 * ((b + c))));
  ans = min(ans, ((a + b) + c));
  write(ans, cpp_char("\n"));
  return 0;
}
