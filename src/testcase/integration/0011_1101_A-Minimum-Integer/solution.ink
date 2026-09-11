// Translated from solution.cpp.

var f: dynamic = cpp_array(1000001);

func pow(a: dynamic, b: dynamic, MOD: dynamic) -> dynamic
{
  var x: dynamic = 1;
  var y: dynamic = a;
  while ((b > 0))
  {
    if (((b % 2) == 1))
    {
      x = ((x * y));
      if ((x > MOD))
      {
        x %= MOD;
      }
    }
    y = ((y * y));
    if ((y > MOD))
    {
      y %= MOD;
    }
    b /= 2;
  }
  return x;
}

func InverseEuler(n: dynamic, MOD: dynamic) -> dynamic
{
  return pow(n, (MOD - 2), MOD);
}

func C(n: dynamic, r: dynamic, MOD: dynamic) -> dynamic
{
  return (((f[n] * ((((InverseEuler(f[r], MOD) * InverseEuler(f[(n - r)], MOD))) % MOD)))) % MOD);
}

func main() -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  read(q);
  while (cpp_update(q, "--"))
  {
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    var d: dynamic = cpp_uninitialized();
    read(l, r, d);
    var ans1: dynamic = d;
    var y: dynamic = (r / d);
    var ans2: dynamic = (((y + 1)) * d);
    if ((d >= l))
    {
      write(ans2, "\n");
    } else
    {
      write(ans1, "\n");
    }
  }
}
