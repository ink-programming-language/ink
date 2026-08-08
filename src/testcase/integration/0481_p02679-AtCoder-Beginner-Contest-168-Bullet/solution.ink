// Translated from solution.cpp.

var MOD = cpp_expression("#include <");

var M: dynamic;

var pow2 = [1];

func gcd(x: dynamic, y: dynamic)
{
  return if (y) gcd(y, (x % y)) else x;
}

func main(argument_0: dynamic)
{
  var n: dynamic;
  var i: dynamic;
  var x: dynamic;
  var y: dynamic;
  var tmp = -1;
  var cnt = 1;
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
    var GCD = gcd(abs(x), abs(y));
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
  for (var p in M)
  {
    cnt = ((cnt * (((pow2[p.second.first] + pow2[p.second.second]) - 1))) % MOD);
  }
  printf("%lld", ((((tmp + MOD) + cnt)) % MOD));
  return 0;
}
