// Translated from solution.cpp.

var ff = cpp_expression("#incl");

var ss = cpp_expression("#inclu");

var pb = cpp_expression("#include<");

var int_cpp = dynamic;

var mp = cpp_expression("#include<");

var pr = cpp_expression("#include<bits/stdc++.h> u");

var vr = cpp_expression("#include<bits/std");

var MOD = cpp_expression("#include<b");

var mod = cpp_expression("#include<b");

var mod2 = cpp_expression("#include<");

var inf = cpp_expression("#inc");

func ps(x: dynamic, y: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h> u");
}

func mk(arr: dynamic, n: dynamic, type_cpp: dynamic)
{
  cpp_macro("type *arr=new type[n];");
}

var ll = dynamic;

var ld = dynamic;

func w(x: dynamic)
{
  cpp_macro("int x; cin>>x; while(x--)");
}

func fill(a: dynamic, b: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h>");
}

var ios = cpp_expression("#include<bits/stdc++.h> using nam");

var spf = cpp_array(1000002);

var N = 200005;

var NN = (5e6 + 5);

var ans = 1e18;

func sieve()
{
  {
    var i = 0;
    while ((i < 5000002))
    {
      spf[i] = i;
      i += 1;
    }
  }
  {
    var i = 2;
    while ((i < 5000002))
    {
      spf[i] = 2;
      i = (i + 2);
    }
  }
  {
    var i = 3;
    while (((i * i) < 5000002))
    {
      if ((spf[i] == i))
      {
        spf[i] = i;
        {
          var j = (i * i);
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

func power(x: dynamic, y: dynamic)
{
  var res = 1;
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

func modInv(a: dynamic)
{
  return power(a, (MOD - 2));
}

var fact = cpp_array(N);

var inv = cpp_array(N);

func factorial(n: dynamic)
{
  fact[0] = 1;
  {
    var i = 1;
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

func InvFactorial(n: dynamic)
{
  inv[0] = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      inv[i] = modInv(fact[i]);
      i += 1;
    }
  }
}

func ncr(n: dynamic, r: dynamic)
{
  if ((((n < r) || (n < 0)) || (r < 0)))
  {
    return 0;
  }
  var b = inv[(n - r)];
  var c = inv[r];
  var a = (fact[n] * b);
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

func isPrime(n: dynamic)
{
  if ((n <= 1))
  {
    return false;
  }
  {
    var i = 2;
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

func isPerfectSquare(x: dynamic)
{
  if ((x >= 0))
  {
    var sr = sqrt(x);
    return (((sr * sr) == x));
  }
  return false;
}

func comparator(a: dynamic, b: dynamic)
{
  return (a < b);
}

func main()
{
  var t: dynamic;
  read(t);
  {
    var j = 0;
    while ((j < t))
    {
      var n: dynamic;
      read(n);
      var b = cpp_array(n);
      var a = cpp_array(n);
      {
        var i = 0;
        while ((i < n))
        {
          read(a[i]);
          b[i] = 0;
          i += 1;
        }
      }
      var sum = 0;
      {
        var i = (n - 1);
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
        var i = 0;
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
