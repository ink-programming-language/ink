// Translated from solution.cpp.

func bexp(a: dynamic, x: dynamic, p: dynamic) -> dynamic
{
  if ((x == 0))
  {
    return 1;
  }
  if (((x % 2) == 1))
  {
    return ((a * bexp(a, (x - 1), p)) % p);
  }
  var t: dynamic = bexp(a, (x / 2), p);
  return ((t * t) % p);
}

func inv(a: dynamic, p: dynamic) -> dynamic
{
  return bexp(a, (p - 2), p);
}

func main() -> dynamic
{
  var p: dynamic = cpp_uninitialized();
  read(p);
  var a: dynamic = cpp_array(p);
  {
    var i: dynamic = 0;
    while ((i < p))
    {
      read(a[i]);
      i += 1;
    }
  }
  var c: dynamic = cpp_array(p, p);
  {
    var i: dynamic = 0;
    while ((i < p))
    {
      c[i][0] = cpp_assign(c[i][i], "=", 1);
      {
        var j: dynamic = 1;
        while ((j < i))
        {
          c[i][j] = (((c[(i - 1)][(j - 1)] + c[(i - 1)][j])) % p);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var e: dynamic = cpp_array(p, p);
  {
    var i: dynamic = 0;
    while ((i < p))
    {
      {
        var j: dynamic = 0;
        while ((j < p))
        {
          e[i][j] =  ((j == 0)) ? 1 : ((e[i][(j - 1)] * i) % p);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var fac: dynamic = cpp_array(p);
  {
    var i: dynamic = 0;
    while ((i < p))
    {
      fac[i] =  ((i == 0)) ? 1 : ((fac[(i - 1)] * i) % p);
      i += 1;
    }
  }
  var b: dynamic = cpp_array(p);
  {
    var i: dynamic = (p - 1);
    while ((i >= 0))
    {
      var x: dynamic = 0;
      var y: dynamic = 0;
      var neg: dynamic = 1;
      {
        var j: dynamic = i;
        while ((j >= 0))
        {
          y = ((((((y + ((a[j] * c[i][j]) * neg))) % p) + p)) % p);
          neg = (-neg);
          j -= 1;
        }
      }
      b[i] = ((y * inv(fac[i], p)) % p);
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < p))
    {
      write(b[i], ( ((i == (p - 1))) ? cpp_char("\n") : cpp_char(" ")));
      i += 1;
    }
  }
  return 0;
}
