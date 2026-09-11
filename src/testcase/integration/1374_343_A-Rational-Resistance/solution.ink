// Translated from solution.cpp.

func f(name: dynamic, arg1: dynamic) -> dynamic
{
  write(name, " : ", arg1, cpp_char("\n"));
}

func f(names: dynamic, arg1: dynamic, args: dynamic...) -> dynamic
{
  var comma: dynamic = strchr((names + 1), cpp_char(","));
  (((cerr.write(names, (comma - names)) << " : ") << arg1) << " | ");
  f((comma + 1), cpp_expand(args));
}

var maxn: dynamic = ((2 * cpp_cast(1e5)) + 10);

var EPS: dynamic = 1e-9;

var INF: dynamic = (cpp_cast(1e18) + 18);

var mod: dynamic = (cpp_cast(1e9) + 9);

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var ans: dynamic = 0;

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return ( (b) ? gcd(b, (a % b)) : a);
}

func main() -> dynamic
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
