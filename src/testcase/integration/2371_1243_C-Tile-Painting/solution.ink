// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  return if ((b == 0)) a else gcd(b, (a % b));
}

func lcm(a: dynamic, b: dynamic)
{
  return (a * ((b / gcd(a, b))));
}

var N = 100005;

func solve()
{
  var n: dynamic;
  read(n);
  var ans = n;
  {
    var i = 2;
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var TESTS = 1;
  while (cpp_update(TESTS, "--"))
  {
    solve();
  }
  write("\nTime elapsed: ", ((1000 * clock()) / CLOCKS_PER_SEC), "ms\n");
  return 0;
}
