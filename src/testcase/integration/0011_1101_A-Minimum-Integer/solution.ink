// Translated from solution.cpp.

var f = cpp_array(1000001);

func pow(a: dynamic, b: dynamic, MOD: dynamic)
{
  var x = 1;
  var y = a;
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

func InverseEuler(n: dynamic, MOD: dynamic)
{
  return pow(n, (MOD - 2), MOD);
}

func C(n: dynamic, r: dynamic, MOD: dynamic)
{
  return (((f[n] * ((((InverseEuler(f[r], MOD) * InverseEuler(f[(n - r)], MOD))) % MOD)))) % MOD);
}

func main()
{
  var q: dynamic;
  read(q);
  while (cpp_update(q, "--"))
  {
    var l: dynamic;
    var r: dynamic;
    var d: dynamic;
    read(l, r, d);
    var ans1 = d;
    var y = (r / d);
    var ans2 = (((y + 1)) * d);
    if ((d >= l))
    {
      write(ans2, "\n");
    } else
    {
      write(ans1, "\n");
    }
  }
}
