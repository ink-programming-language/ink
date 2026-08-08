// Translated from solution.cpp.

var ll = dynamic;

var pb = cpp_expression("#include");

var CR7 = cpp_expression("#include <bits/stdc++.h>");

var pii = cpp_expression("#include <bi");

var MOD = cpp_expression("#include <");

var vi = cpp_expression("#include <");

var vii = cpp_expression("#include <b");

var mi = cpp_expression("#include <b");

var a = cpp_array(100000);

var k: dynamic;

var N = 1005;

var fact = cpp_array(N);

func mod_pow(a: dynamic, n: dynamic, mod: dynamic)
{
  if ((n == 0))
  {
    return 1;
  }
  var res = mod_pow((((a * a)) % mod), (n / 2), mod);
  if ((n % 2))
  {
    res = (((res * a)) % mod);
  }
  return res;
}

func mod_inv(x: dynamic, mod: dynamic)
{
  return mod_pow(x, (mod - 2), mod);
}

func nCr(n: dynamic, r: dynamic, mod: dynamic)
{
  return ((((((fact[n] * mod_inv(fact[r], mod)) % mod)) * ((mod_inv(fact[(n - r)], mod) % mod)))) % mod);
}

func factorial(mod: dynamic)
{
  fact[0] = 1;
  {
    var i = 1;
    while ((i < N))
    {
      fact[i] = ((i * fact[(i - 1)]));
      fact[i] %= mod;
      i += 1;
    }
  }
}

func main()
{
  CR7;
  var t: dynamic;
  read(t);
  factorial(MOD);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n, k);
    var b = cpp_array(n);
    var ma: dynamic;
    var mk: dynamic;
    {
      var i = 0;
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
    var sum = 0;
    {
      var i = 0;
      while ((i < k))
      {
        sum += b[i];
        mk[b[i]] += 1;
        i += 1;
      }
    }
    var mxans = 1;
    for (var g in mk)
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
