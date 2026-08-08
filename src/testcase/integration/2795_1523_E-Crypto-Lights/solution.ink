// Translated from solution.cpp.

var cpp_name = cpp_expression("#include <bits/stdc++.h> usi");

var endl = cpp_expression("#inc");

var MOD = (1e9 + 7);

var MAX = (1e5 + 10);

func inv(a: dynamic, b: dynamic = MOD)
{
  return if ((a > 1)) (b - ((inv((b % a), a) * b) / a)) else 1;
}

var fat = cpp_array(MAX);

func choose(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    return 0;
  }
  return ((((fat[a] * inv(fat[b])) % MOD) * inv(fat[(a - b)])) % MOD);
}

func main()
{
  var fat = 1;
  {
    var i = 1;
    while ((i < MAX))
    {
      fat[i] = ((i * fat[(i - 1)]) % MOD);
      i += 1;
    }
  }
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var k: dynamic;
    read(n, k);
    var ans = 0;
    {
      var i = 1;
      while ((i <= n))
      {
        var den = choose(n, i);
        var valid = choose((n - (((k - 1)) * ((i - 1)))), i);
        ans = (((ans + ((valid * inv(den)) % MOD))) % MOD);
        i += 1;
      }
    }
    write((((ans + 1)) % MOD), "\n");
  }
  exit(0);
}
