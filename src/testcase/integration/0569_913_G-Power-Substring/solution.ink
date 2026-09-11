// Translated from solution.cpp.

var T: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

func ps(x: dynamic, y: dynamic, P: dynamic) -> dynamic
{
  var z: dynamic = 0;
  while (y)
  {
    if ((y & 1))
    {
      z = (z + x);
    }
    x <<= 1;
    y >>= 1;
    if ((x >= P))
    {
      x -= P;
    }
    if ((z >= P))
    {
      z -= P;
    }
  }
  return z;
}

func pm(x: dynamic, y: dynamic, P: dynamic) -> dynamic
{
  var z: dynamic = 1;
  x %= P;
  while (y)
  {
    if ((y & 1))
    {
      z = ps(z, x, P);
    }
    x = ps(x, x, P);
    y >>= 1;
  }
  return z;
}

func main() -> dynamic
{
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    scanf("%lld", (&a));
    var t: dynamic = a;
    var m1: dynamic = 1;
    n = 0;
    while (t)
    {
      t /= 10;
      n += 1;
    }
    {
      m = 0;
      while (true)
      {
        b = (((-a)) & ((((1 << ((n + m)))) - 1)));
        if (((((a + b)) % 5) == 0))
        {
          b += (1 << ((n + m)));
        }
        if ((b >= m1))
        {
          m += 1;
          a *= 10;
          m1 *= 10;
          continue;
        }
        x = (a + b);
        y = (x >> ((n + m)));
        var i: dynamic = cpp_uninitialized();
        var j: dynamic = cpp_uninitialized();
        var now: dynamic = 0;
        var phi: dynamic = cpp_uninitialized();
        var pw: dynamic = cpp_uninitialized();
        {
          i = 0;
          while ((i < 4))
          {
            if ((pm(2, i, 5) == (y % 5)))
            {
              now = i;
            }
            i += 1;
          }
        }
        phi = 4;
        pw = 5;
        {
          i = 2;
          while ((i <= (n + m)))
          {
            pw *= 5;
            {
              j = 0;
              while ((j < 5))
              {
                if ((pm(2, (now + (j * phi)), pw) == (y % pw)))
                {
                  now += (j * phi);
                  break;
                }
                j += 1;
              }
            }
            phi *= 5;
            i += 1;
          }
        }
        printf("%lld\n", ((now + n) + m));
        break;
        m += 1;
        a *= 10;
        m1 *= 10;
      }
    }
  }
  return 0;
}
