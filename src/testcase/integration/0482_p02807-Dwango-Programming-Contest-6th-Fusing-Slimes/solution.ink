// Translated from solution.cpp.

var mod = 1000000007;

func mi(x: dynamic, m: dynamic)
{
  var acc = 1;
  var p = (m - 2);
  while (p)
  {
    if (((p % 2) == 1))
    {
      acc *= x;
      acc %= m;
    }
    x *= (x % m);
    x %= m;
    p >>= 1;
  }
  return acc;
}

func main()
{
  var n: dynamic;
  read(n);
  for (var e in x)
  {
    read(e);
  }
  var mijs = cpp_construct((n - 1));
  mijs[0] = 1;
  {
    var i = 1;
    while ((i < (n - 1)))
    {
      mijs[i] = (mijs[(i - 1)] + mi((i + 1), mod));
      mijs[i] %= mod;
      i += 1;
    }
  }
  var sum = 0;
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      sum += ((((x[(i + 1)] - x[i])) * mijs[i]) % mod);
      sum %= mod;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= (n - 1)))
    {
      sum *= i;
      sum %= mod;
      i += 1;
    }
  }
  write(sum, "\n");
}
