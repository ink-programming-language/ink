// Translated from solution.cpp.

var cpp_name: dynamic = cpp_expression("#include <bits/stdc++.h> usi");

var endl: dynamic = cpp_expression("#inc");

var MOD: dynamic = (1e9 + 7);

var MAX: dynamic = (1e5 + 10);

func inv(a: dynamic, b: dynamic = MOD) -> dynamic
{
  return  ((a > 1)) ? (b - ((inv((b % a), a) * b) / a)) : 1;
}

var fat: dynamic = cpp_array(MAX);

func choose(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    return 0;
  }
  return ((((fat[a] * inv(fat[b])) % MOD) * inv(fat[(a - b)])) % MOD);
}

func main() -> dynamic
{
  var fat: dynamic = 1;
  {
    var i: dynamic = 1;
    while ((i < MAX))
    {
      fat[i] = ((i * fat[(i - 1)]) % MOD);
      i += 1;
    }
  }
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var k: dynamic = cpp_uninitialized();
    read(n, k);
    var ans: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        var den: dynamic = choose(n, i);
        var valid: dynamic = choose((n - (((k - 1)) * ((i - 1)))), i);
        ans = (((ans + ((valid * inv(den)) % MOD))) % MOD);
        i += 1;
      }
    }
    write((((ans + 1)) % MOD), "\n");
  }
  exit(0);
}
