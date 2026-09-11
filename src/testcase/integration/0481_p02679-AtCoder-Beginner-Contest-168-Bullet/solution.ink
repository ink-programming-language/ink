// Translated from solution.cpp.

var MOD: dynamic = cpp_expression("#include <");

var M: dynamic = cpp_uninitialized();

var pow2: dynamic = [1];

func gcd(x: dynamic, y: dynamic) -> dynamic
{
  return  (y) ? gcd(y, (x % y)) : x;
}

func main(argument_0: dynamic) -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var tmp: dynamic = -1;
  var cnt: dynamic = 1;
  scanf("%lld", (&n));
  while (cpp_update(n, "--"))
  {
    scanf("%lld %lld", (&x), (&y));
    if (((x == 0) && (y == 0)))
    {
      tmp += 1;
      continue;
    }
    if ((make_pair(x, y) < make_pair(0, 0)))
    {
      x = (-x);
      y = (-y);
    }
    var GCD: dynamic = gcd(abs(x), abs(y));
    x /= GCD;
    y /= GCD;
    if ((y > 0))
    {
      M[make_pair(x, y)].first += 1;
    } else
    {
      M[make_pair((-y), x)].second += 1;
    }
  }
  {
    i = 1;
    while ((i < 222222))
    {
      pow2[i] = ((pow2[(i - 1)] * 2) % MOD);
      i += 1;
    }
  }
  for (var p: dynamic in M)
  {
    cnt = ((cnt * (((pow2[p.second.first] + pow2[p.second.second]) - 1))) % MOD);
  }
  printf("%lld", ((((tmp + MOD) + cnt)) % MOD));
  return 0;
}
