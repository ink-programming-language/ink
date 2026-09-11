// Translated from solution.cpp.

var ll: dynamic = dynamic;

var pb: dynamic = cpp_expression("#include");

var CR7: dynamic = cpp_expression("#include <bits/stdc++.h>");

var pii: dynamic = cpp_expression("#include <bi");

var MOD: dynamic = cpp_expression("#include <");

var vi: dynamic = cpp_expression("#include <");

var vii: dynamic = cpp_expression("#include <b");

var mi: dynamic = cpp_expression("#include <b");

var a: dynamic = cpp_array(100000);

var k: dynamic = cpp_uninitialized();

var N: dynamic = 1005;

var fact: dynamic = cpp_array(N);

func mod_pow(a: dynamic, n: dynamic, mod: dynamic) -> dynamic
{
  if ((n == 0))
  {
    return 1;
  }
  var res: dynamic = mod_pow((((a * a)) % mod), (n / 2), mod);
  if ((n % 2))
  {
    res = (((res * a)) % mod);
  }
  return res;
}

func mod_inv(x: dynamic, mod: dynamic) -> dynamic
{
  return mod_pow(x, (mod - 2), mod);
}

func nCr(n: dynamic, r: dynamic, mod: dynamic) -> dynamic
{
  return ((((((fact[n] * mod_inv(fact[r], mod)) % mod)) * ((mod_inv(fact[(n - r)], mod) % mod)))) % mod);
}

func factorial(mod: dynamic) -> dynamic
{
  fact[0] = 1;
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      fact[i] = ((i * fact[(i - 1)]));
      fact[i] %= mod;
      i += 1;
    }
  }
}

func main() -> dynamic
{
  CR7;
  var t: dynamic = cpp_uninitialized();
  read(t);
  factorial(MOD);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n, k);
    var b: dynamic = cpp_array(n);
    var ma: dynamic = cpp_uninitialized();
    var mk: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(a[i]);
        b[i] = a[i];
        ma[a[i]] += 1;
        i += 1;
      }
    }
    sort(b, (b + n));
    reverse(b, (b + n));
    var sum: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < k))
      {
        sum += b[i];
        mk[b[i]] += 1;
        i += 1;
      }
    }
    var mxans: dynamic = 1;
    for (var g: dynamic in mk)
    {
      mxans = ((((((mxans % MOD)) * nCr(ma[g.first], g.second, MOD)) % MOD)) % MOD);
      if ((mxans < 0))
      {
        mxans += MOD;
      }
    }
    if ((mxans < 0))
    {
      mxans += MOD;
    }
    write((mxans % MOD), "\n");
  }
  return 0;
}
