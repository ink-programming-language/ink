// Translated from solution.cpp.

var N = 200010;

var Mod = 1000000007;

var n: dynamic;

var T: dynamic;

var M: dynamic;

var res: dynamic;

var ans: dynamic;

var a = cpp_array(N);

var inv = cpp_array(N);

var fac = cpp_array(N);

var Inv = cpp_array(N);

var pow_2 = cpp_array(N);

func read()
{
  var x = 0;
  var f = 1;
  var ch = getchar();
  while ((!isdigit(ch)))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (isdigit(ch))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (x * f);
}

func C(n: dynamic, m: dynamic)
{
  if ((n < m))
  {
    return 0;
  }
  return ((((fac[n] * inv[m]) % Mod) * inv[(n - m)]) % Mod);
}

func Power(base: dynamic, power: dynamic)
{
  var result = 1;
  while ((power > 0))
  {
    if ((power & 1))
    {
      result = ((result * base) % Mod);
    }
    power >>= 1;
    base = (((base * base)) % Mod);
  }
  return result;
}

func Pre()
{
  fac[0] = cpp_assign(inv[0], "=", cpp_assign(pow_2[0], "=", cpp_assign(Inv[0], "=", 1)));
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] = read();
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] += a[(i - 1)];
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      fac[i] = ((fac[(i - 1)] * i) % Mod);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      inv[i] = Power(fac[i], (Mod - 2));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      pow_2[i] = ((pow_2[(i - 1)] * 2) % Mod);
      Inv[i] = Power(pow_2[i], (Mod - 2));
      i += 1;
    }
  }
}

func main()
{
  n = read();
  T = read();
  Pre();
  M = n;
  res = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      var s = min(n, (T - a[i]));
      if ((s < 0))
      {
        i += 1;
        continue;
      }
      res = (((((((2 * res) - C((i - 1), M))) % Mod) + Mod)) % Mod);
      while ((M > s))
      {
        res = ((((((res - C(i, M))) % Mod) + Mod)) % Mod);
        M -= 1;
      }
      ans = (((ans + ((res * Inv[i]) % Mod))) % Mod);
      i += 1;
    }
  }
  printf("%lld\n", (ans % Mod));
  return 0;
}
