// Translated from solution.cpp.

var mod = 1000000007;

func powmod(a: dynamic, b: dynamic)
{
  var res = 1;
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

func gcd(a: dynamic, b: dynamic)
{
  return if (b) gcd(b, (a % b)) else a;
}

func getint()
{
  var ret = 0;
  var ok = 0;
  var neg = 0;
  {
    while (true)
    {
      var c = getchar();
      if (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
      {
        ret = ((((((ret << 3)) + ret) + ret) + c) - cpp_char("0"));
        ok = 1;
      } else if (ok)
      {
        return if (neg) (-ret) else ret;
      } else if ((c == cpp_char("-")))
      {
        neg = 1;
      }
    }
  }
}

var m: dynamic;

var d: dynamic;

var n: dynamic;

var base = cpp_array(10, 10);

var a = cpp_array(10);

var hs: dynamic;

func main()
{
  m = getint();
  d = getint();
  {
    var zz = 0;
    while ((zz < m))
    {
      var c = getint();
      {
        var j = 0;
        while ((j < d))
        {
          {
            var k = 0;
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
        var z = 0;
        while ((z < c))
        {
          {
            var k = 0;
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
            var j = 0;
            while ((j < d))
            {
              if ((!a[j]))
              {
                j += 1;
                continue;
              }
              if (base[j][j])
              {
                var w = (mod - a[j]);
                {
                  var k = j;
                  while ((k < d))
                  {
                    a[k] = (((a[k] + (w * base[j][k]))) % mod);
                    k += 1;
                  }
                }
              } else
              {
                var w = powmod(a[j], (mod - 2));
                {
                  var k = j;
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
        var j = 0;
        while ((j < d))
        {
          if (base[j][j])
          {
            {
              var i = 0;
              while ((i < j))
              {
                if (base[i][j])
                {
                  var w = (mod - base[i][j]);
                  {
                    var k = j;
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
      var p = 0;
      {
        var j = 0;
        while ((j < d))
        {
          {
            var k = 0;
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
