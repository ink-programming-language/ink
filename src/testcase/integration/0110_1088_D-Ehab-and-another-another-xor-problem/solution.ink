// Translated from solution.cpp.

func fpow(n: dynamic, k: dynamic, p: dynamic = 998244353)
{
  var r = 1;
  {
    while (k)
    {
      if ((k & 1))
      {
        r = ((r * n) % p);
      }
      n = ((n * n) % p);
      k >>= 1;
    }
  }
  return r;
}

func inv(a: dynamic, p: dynamic = 998244353)
{
  return fpow(a, (p - 2), p);
}

func addmod(a: dynamic, val: dynamic, p: dynamic = 998244353)
{
  {
    if (((cpp_assign(a, "=", ((a + val)))) >= p))
    {
      a -= p;
    }
  }
  return a;
}

func submod(a: dynamic, val: dynamic, p: dynamic = 998244353)
{
  {
    if (((cpp_assign(a, "=", ((a - val)))) < 0))
    {
      a += p;
    }
  }
  return a;
}

func mult(a: dynamic, b: dynamic, p: dynamic = 998244353)
{
  return ((cpp_cast(a) * b) % p);
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var greater = false;
  write("? 0 0", "\n");
  var x: dynamic;
  var y: dynamic;
  read(x);
  if ((x == 1))
  {
    greater = true;
  }
  var cura = 0;
  var curb = 0;
  {
    var i = 29;
    while ((i >= 0))
    {
      write("? ", ((cura ^ ((1 << i)))), " ", curb, "\n");
      read(x);
      write("? ", cura, " ", ((curb ^ ((1 << i)))), "\n");
      read(y);
      if ((x != y))
      {
        if ((y == 1))
        {
          cura |= ((1 << i));
          curb |= ((1 << i));
        }
      } else
      {
        if (greater)
        {
          cura |= ((1 << i));
        } else
        {
          curb |= ((1 << i));
        }
        if ((x == 1))
        {
          greater = true;
        } else
        {
          greater = false;
        }
      }
      i -= 1;
    }
  }
  write("! ", cura, " ", curb, "\n");
}
