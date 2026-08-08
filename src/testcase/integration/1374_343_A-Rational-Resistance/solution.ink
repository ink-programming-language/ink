// Translated from solution.cpp.

func f(name: dynamic, arg1: dynamic)
{
  write(name, " : ", arg1, cpp_char("\n"));
}

func f(names: dynamic, arg1: dynamic, args: dynamic...)
{
  var comma = strchr((names + 1), cpp_char(","));
  (((cerr.write(names, (comma - names)) << " : ") << arg1) << " | ");
  f((comma + 1), cpp_expand(args));
}

var maxn = ((2 * cpp_cast(1e5)) + 10);

var EPS = 1e-9;

var INF = (cpp_cast(1e18) + 18);

var mod = (cpp_cast(1e9) + 9);

var a: dynamic;

var b: dynamic;

var ans = 0;

func gcd(a: dynamic, b: dynamic)
{
  return (if (b) gcd(b, (a % b)) else a);
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(a, b);
  if ((a < b))
  {
    swap(a, b);
  }
  while ((b > 0))
  {
    ans += ((a / b));
    a %= b;
    if ((a < b))
    {
      swap(a, b);
    }
  }
  write(ans, cpp_char("\n"));
  return 0;
}
