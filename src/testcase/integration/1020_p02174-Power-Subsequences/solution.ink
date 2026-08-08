// Translated from solution.cpp.

var mod = 998244353;

var inf = (((1 << 30)) - 1);

var infll = (((1 << 61)) - 1);

func fast()
{
  return cpp_expression("#include <bits/stdc++.h> using namespac");
}

func mod_pow(x: dynamic, n: dynamic, mod: dynamic)
{
  var res = 1;
  while ((n > 0))
  {
    if ((n & 1))
    {
      (cpp_assign(res, "*=", x)) %= mod;
    }
    (cpp_assign(x, "*=", x)) %= mod;
    n >>= 1;
  }
  return res;
}

var N: dynamic;

func main()
{
  read(N);
  var x = cpp_construct((N + 1));
  {
    var i = 0;
    while ((i < N))
    {
      read(x[(i + 1)]);
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 1;
    while ((i <= N))
    {
      ans += ((((mod_pow(2, (N - i), mod) * x[i]) % mod) * mod_pow((x[i] + 1), (i - 1), mod)) % mod);
      ans %= mod;
      i += 1;
    }
  }
  write((ans % mod), "\n");
}
