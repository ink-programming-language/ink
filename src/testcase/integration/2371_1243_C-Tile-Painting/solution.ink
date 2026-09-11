// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  ((b == 0)) ? a : gcd(b, (a % b));
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return (a * ((b / gcd(a, b))));
}

var N: dynamic = 100005;

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var ans: dynamic = n;
  {
    var i: dynamic = 2;
    while (((i * i) <= n))
    {
      if (((n % i) == 0))
      {
        ans = gcd(ans, i);
        ans = gcd(ans, (n / i));
      }
      i += 1;
    }
  }
  write(ans);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var TESTS: dynamic = 1;
  while (cpp_update(TESTS, "--"))
  {
    solve();
  }
  write("\nTime elapsed: ", ((1000 * clock()) / CLOCKS_PER_SEC), "ms\n");
  return 0;
}
