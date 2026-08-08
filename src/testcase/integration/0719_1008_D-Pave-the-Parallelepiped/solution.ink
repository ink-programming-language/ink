// Translated from solution.cpp.

var MAXN = (1e5 + 10);

var fac = cpp_array(MAXN);

var ft = cpp_array(8);

var C = cpp_array(1000, 1000);

var num = cpp_array(8);

func Init()
{
  {
    var i = 1;
    while ((i < MAXN))
    {
      {
        var j = i;
        while ((j < MAXN))
        {
          fac[j] += 1;
          j += i;
        }
      }
      i += 1;
    }
  }
  memset(C, 0, cpp_sizeof((C)));
  {
    var i = 0;
    while ((i < 1000))
    {
      C[i][0] = 1;
      {
        var j = 1;
        while ((j <= i))
        {
          C[i][j] = (C[(i - 1)][(j - 1)] + C[(i - 1)][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func check(a: dynamic, b: dynamic, c: dynamic)
{
  if (((((a & 1)) && ((((b >> 1)) & 1))) && ((((c >> 2)) & 1))))
  {
    return true;
  }
  if (((((a & 1)) && ((((c >> 1)) & 1))) && ((((b >> 2)) & 1))))
  {
    return true;
  }
  if (((((b & 1)) && ((((a >> 1)) & 1))) && ((((c >> 2)) & 1))))
  {
    return true;
  }
  if (((((b & 1)) && ((((c >> 1)) & 1))) && ((((a >> 2)) & 1))))
  {
    return true;
  }
  if (((((c & 1)) && ((((a >> 1)) & 1))) && ((((b >> 2)) & 1))))
  {
    return true;
  }
  if (((((c & 1)) && ((((b >> 1)) & 1))) && ((((a >> 2)) & 1))))
  {
    return true;
  }
  return false;
}

func gcd(a: dynamic, b: dynamic)
{
  return if (b) gcd(b, (a % b)) else a;
}

func main()
{
  Init();
  var T: dynamic;
  scanf("%d", (&T));
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var gcdab: dynamic;
  var gcdac: dynamic;
  var gcdbc: dynamic;
  var gcdabc: dynamic;
  while (cpp_update(T, "--"))
  {
    scanf("%d%d%d", (&a), (&b), (&c));
    gcdab = gcd(a, b);
    gcdac = gcd(a, c);
    gcdbc = gcd(b, c);
    gcdabc = gcd(gcdab, c);
    ft[1] = (((fac[a] - fac[gcdab]) - fac[gcdac]) + fac[gcdabc]);
    ft[2] = (((fac[b] - fac[gcdab]) - fac[gcdbc]) + fac[gcdabc]);
    ft[4] = (((fac[c] - fac[gcdac]) - fac[gcdbc]) + fac[gcdabc]);
    ft[3] = (fac[gcdab] - fac[gcdabc]);
    ft[5] = (fac[gcdac] - fac[gcdabc]);
    ft[6] = (fac[gcdbc] - fac[gcdabc]);
    ft[7] = fac[gcdabc];
    var ans = 0;
    {
      var i = 1;
      while ((i < 8))
      {
        {
          var j = i;
          while ((j < 8))
          {
            {
              var k = j;
              while ((k < 8))
              {
                if (check(i, j, k))
                {
                  memset(num, 0, cpp_sizeof((num)));
                  num[i] += 1;
                  num[j] += 1;
                  num[k] += 1;
                  var temp = 1;
                  var flag = 0;
                  {
                    var l = 1;
                    while ((l < 8))
                    {
                      if (num[l])
                      {
                        if ((((ft[l] + num[l]) - 1) > 0))
                        {
                          temp *= C[((ft[l] + num[l]) - 1)][num[l]];
                        } else
                        {
                          flag = 1;
                        }
                      }
                      l += 1;
                    }
                  }
                  if ((!flag))
                  {
                    ans += temp;
                  }
                }
                k += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    printf("%lld\n", ans);
  }
  return 0;
}
