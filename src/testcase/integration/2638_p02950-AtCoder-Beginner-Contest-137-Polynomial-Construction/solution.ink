// Translated from solution.cpp.

func bexp(a: dynamic, x: dynamic, p: dynamic)
{
  if ((x == 0))
  {
    return 1;
  }
  if (((x % 2) == 1))
  {
    return ((a * bexp(a, (x - 1), p)) % p);
  }
  var t = bexp(a, (x / 2), p);
  return ((t * t) % p);
}

func inv(a: dynamic, p: dynamic)
{
  return bexp(a, (p - 2), p);
}

func main()
{
  var p: dynamic;
  read(p);
  var a = cpp_array(p);
  {
    var i = 0;
    while ((i < p))
    {
      read(a[i]);
      i += 1;
    }
  }
  var c = cpp_array(p, p);
  {
    var i = 0;
    while ((i < p))
    {
      c[i][0] = cpp_assign(c[i][i], "=", 1);
      {
        var j = 1;
        while ((j < i))
        {
          c[i][j] = (((c[(i - 1)][(j - 1)] + c[(i - 1)][j])) % p);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var e = cpp_array(p, p);
  {
    var i = 0;
    while ((i < p))
    {
      {
        var j = 0;
        while ((j < p))
        {
          e[i][j] = if ((j == 0)) 1 else ((e[i][(j - 1)] * i) % p);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var fac = cpp_array(p);
  {
    var i = 0;
    while ((i < p))
    {
      fac[i] = if ((i == 0)) 1 else ((fac[(i - 1)] * i) % p);
      i += 1;
    }
  }
  var b = cpp_array(p);
  {
    var i = (p - 1);
    while ((i >= 0))
    {
      var x = 0;
      var y = 0;
      var neg = 1;
      {
        var j = i;
        while ((j >= 0))
        {
          y = ((((((y + ((a[j] * c[i][j]) * neg))) % p) + p)) % p);
          neg = (-neg);
          j -= 1;
        }
      }
      b[i] = ((y * inv(fac[i], p)) % p);
      {
        var j = 0;
        while ((j < p))
        {
          a[j] = ((((((a[j] - (b[i] * e[j][i]))) % p) + p)) % p);
          j += 1;
        }
      }
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i < p))
    {
      write(b[i], (if ((i == (p - 1))) cpp_char("\n") else cpp_char(" ")));
      i += 1;
    }
  }
  return 0;
}
