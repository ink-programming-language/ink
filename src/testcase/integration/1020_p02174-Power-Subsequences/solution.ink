// Translated from solution.cpp.

var mod: dynamic = 998244353;

var inf: dynamic = (((1 << 30)) - 1);

var infll: dynamic = (((1 << 61)) - 1);

func fast() -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> using namespac");
}

func mod_pow(x: dynamic, n: dynamic, mod: dynamic) -> dynamic
{
  var res: dynamic = 1;
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

var N: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(N);
  var x: dynamic = cpp_construct((N + 1));
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(x[(i + 1)]);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      ans += ((((mod_pow(2, (N - i), mod) * x[i]) % mod) * mod_pow((x[i] + 1), (i - 1), mod)) % mod);
      ans %= mod;
      i += 1;
    }
  }
  write((ans % mod), "\n");
}
