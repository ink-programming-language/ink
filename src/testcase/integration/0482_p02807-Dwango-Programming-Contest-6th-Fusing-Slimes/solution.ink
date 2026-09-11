// Translated from solution.cpp.

var mod: dynamic = 1000000007;

func mi(x: dynamic, m: dynamic) -> dynamic
{
  var acc: dynamic = 1;
  var p: dynamic = (m - 2);
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

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  for (var e: dynamic in x)
  {
    read(e);
  }
  var mijs: dynamic = cpp_construct((n - 1));
  mijs[0] = 1;
  {
    var i: dynamic = 1;
    while ((i < (n - 1)))
    {
      mijs[i] = (mijs[(i - 1)] + mi((i + 1), mod));
      mijs[i] %= mod;
      i += 1;
    }
  }
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      sum += ((((x[(i + 1)] - x[i])) * mijs[i]) % mod);
      sum %= mod;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= (n - 1)))
    {
      sum *= i;
      sum %= mod;
      i += 1;
    }
  }
  write(sum, "\n");
}
