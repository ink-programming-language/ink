// Translated from solution.cpp.

var mod: dynamic = 1000000007;

func powmod(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = 1;
  a %= mod;
  assert((b >= 0));
  {
    while (b)
    {
      if ((b & 1))
      {
        res = ((res * a) % mod);
      }
      a = ((a * a) % mod);
      b >>= 1;
    }
  }
  return res;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  (b) ? gcd(b, (a % b)) : a;
}

func getint() -> dynamic
{
  var ret: dynamic = 0;
  var ok: dynamic = 0;
  var neg: dynamic = 0;
  {
    while (true)
    {
      var c: dynamic = getchar();
      if (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
      {
        ret = ((((((ret << 3)) + ret) + ret) + c) - cpp_char("0"));
        ok = 1;
      } else if (ok)
      {
        return  (neg) ? (-ret) : ret;
      } else if ((c == cpp_char("-")))
      {
        neg = 1;
      }
    }
  }
}

var m: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var base: dynamic = cpp_array(10, 10);

var a: dynamic = cpp_array(10);

var hs: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  m = getint();
  d = getint();
  {
    var zz: dynamic = 0;
    while ((zz < m))
    {
      var c: dynamic = getint();
      {
        var j: dynamic = 0;
        while ((j < d))
        {
          {
            var k: dynamic = 0;
            while ((k < d))
            {
              base[j][k] = 0;
              k += 1;
            }
          }
          j += 1;
        }
      }
      {
        var z: dynamic = 0;
        while ((z < c))
        {
          {
            var k: dynamic = 0;
            while ((k < d))
            {
              a[k] = getint();
              if ((a[k] < 0))
              {
                a[k] += mod;
              }
              k += 1;
            }
          }
          {
            var j: dynamic = 0;
            while ((j < d))
            {
              if ((!a[j]))
              {
                j += 1;
                continue;
              }
              if (base[j][j])
              {
                var w: dynamic = (mod - a[j]);
                {
                  var k: dynamic = j;
                  while ((k < d))
                  {
                    a[k] = (((a[k] + (w * base[j][k]))) % mod);
                    k += 1;
                  }
                }
              } else
              {
                var w: dynamic = powmod(a[j], (mod - 2));
                {
                  var k: dynamic = j;
                  while ((k < d))
                  {
                    base[j][k] = ((a[k] * w) % mod);
                    k += 1;
                  }
                }
                break;
              }
              j += 1;
            }
          }
          z += 1;
        }
      }
      {
        var j: dynamic = 0;
        while ((j < d))
        {
          if (base[j][j])
          {
            {
              var i: dynamic = 0;
              while ((i < j))
              {
                if (base[i][j])
                {
                  var w: dynamic = (mod - base[i][j]);
                  {
                    var k: dynamic = j;
                    while ((k < d))
                    {
                      base[i][k] = (((base[i][k] + (w * base[j][k]))) % mod);
                      k += 1;
                    }
                  }
                }
                i += 1;
              }
            }
          }
          j += 1;
        }
      }
      var p: dynamic = 0;
      {
        var j: dynamic = 0;
        while ((j < d))
        {
          {
            var k: dynamic = 0;
            while ((k < d))
            {
              p = ((p * 13331) + base[j][k]);
              k += 1;
            }
          }
          j += 1;
        }
      }
      if ((!hs.count(p)))
      {
        hs[p] = cpp_update(n, "++");
      }
      printf("%d ", hs[p]);
      zz += 1;
    }
  }
}
