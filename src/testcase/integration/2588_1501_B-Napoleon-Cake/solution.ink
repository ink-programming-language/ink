// Translated from solution.cpp.

var ff: dynamic = cpp_expression("#incl");

var ss: dynamic = cpp_expression("#inclu");

var pb: dynamic = cpp_expression("#include<");

var int_cpp: dynamic = dynamic;

var mp: dynamic = cpp_expression("#include<");

var pr: dynamic = cpp_expression("#include<bits/stdc++.h> u");

var vr: dynamic = cpp_expression("#include<bits/std");

var MOD: dynamic = cpp_expression("#include<b");

var mod: dynamic = cpp_expression("#include<b");

var mod2: dynamic = cpp_expression("#include<");

var inf: dynamic = cpp_expression("#inc");

func ps(x: dynamic, y: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.h> u");
}

func mk(arr: dynamic, n: dynamic, type_cpp: dynamic) -> dynamic
{
  cpp_macro("type *arr=new type[n];");
}

var ll: dynamic = dynamic;

var ld: dynamic = dynamic;

func w(x: dynamic) -> dynamic
{
  cpp_macro("int x; cin>>x; while(x--)");
}

func fill(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.h>");
}

var ios: dynamic = cpp_expression("#include<bits/stdc++.h> using nam");

var spf: dynamic = cpp_array(1000002);

var N: dynamic = 200005;

var NN: dynamic = (5e6 + 5);

var ans: dynamic = 1e18;

func sieve() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < 5000002))
    {
      spf[i] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 2;
    while ((i < 5000002))
    {
      spf[i] = 2;
      i = (i + 2);
    }
  }
  {
    var i: dynamic = 3;
    while (((i * i) < 5000002))
    {
      if ((spf[i] == i))
      {
        spf[i] = i;
        {
          var j: dynamic = (i * i);
          while ((j < 5000002))
          {
            if ((spf[j] == j))
            {
              spf[j] = i;
            }
            j = (j + i);
          }
        }
      }
      i = (i + 2);
    }
  }
}

func power(x: dynamic, y: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while ((y > 0))
  {
    if ((y & 1))
    {
      res = ((res * x) % mod);
    }
    y = (y >> 1);
    x = ((x * x) % mod);
  }
  return (res % mod);
}

func modInv(a: dynamic) -> dynamic
{
  return power(a, (MOD - 2));
}

var fact: dynamic = cpp_array(N);

var inv: dynamic = cpp_array(N);

func factorial(n: dynamic) -> dynamic
{
  fact[0] = 1;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      fact[i] = (fact[(i - 1)] * i);
      if ((fact[i] >= MOD))
      {
        fact[i] %= MOD;
      }
      i += 1;
    }
  }
}

func InvFactorial(n: dynamic) -> dynamic
{
  inv[0] = 1;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      inv[i] = modInv(fact[i]);
      i += 1;
    }
  }
}

func ncr(n: dynamic, r: dynamic) -> dynamic
{
  if ((((n < r) || (n < 0)) || (r < 0)))
  {
    return 0;
  }
  var b: dynamic = inv[(n - r)];
  var c: dynamic = inv[r];
  var a: dynamic = (fact[n] * b);
  if ((a >= MOD))
  {
    a %= MOD;
  }
  a *= c;
  if ((a >= MOD))
  {
    a %= MOD;
  }
  return a;
}

func isPrime(n: dynamic) -> dynamic
{
  if ((n <= 1))
  {
    return false;
  }
  {
    var i: dynamic = 2;
    while (((i * i) <= n))
    {
      if (((n % i) == 0))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func isPerfectSquare(x: dynamic) -> dynamic
{
  if ((x >= 0))
  {
    var sr: dynamic = sqrt(x);
    return (((sr * sr) == x));
  }
  return false;
}

func comparator(a: dynamic, b: dynamic) -> dynamic
{
  return (a < b);
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  {
    var j: dynamic = 0;
    while ((j < t))
    {
      var n: dynamic = cpp_uninitialized();
      read(n);
      var b: dynamic = cpp_array(n);
      var a: dynamic = cpp_array(n);
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          read(a[i]);
          b[i] = 0;
          i += 1;
        }
      }
      var sum: dynamic = 0;
      {
        var i: dynamic = (n - 1);
        while ((i >= 0))
        {
          sum = max(sum, a[i]);
          if ((sum > 0))
          {
            b[i] = 1;
          }
          sum -= 1;
          i -= 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          write(b[i], " ");
          i += 1;
        }
      }
      write("\n");
      j += 1;
    }
  }
}
