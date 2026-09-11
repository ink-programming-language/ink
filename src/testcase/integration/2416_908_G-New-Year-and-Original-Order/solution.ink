// Translated from solution.cpp.

func getre() -> dynamic
{
  var x: dynamic = 0;
  printf("%d\n", (1 / x));
}

func gettle() -> dynamic
{
  var res: dynamic = 1;
  while (1)
  {
    res <<= 1;
  }
  printf("%d\n", res);
}

func upmin(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a > b)) ? cpp_comma(cpp_assign(a, "=", b), 1) : 0;
}

func upmax(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a < b)) ? cpp_comma(cpp_assign(a, "=", b), 1) : 0;
}

func flo(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a >= 0)) ? (a / b) : ((-(((((-a) - 1)) / b))) - 1);
}

func cei(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a > 0)) ? ((((a - 1)) / b) + 1) : (-(((-a) / b)));
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  (b) ? gcd(b, (a % b)) : a;
}

func sgn(a: dynamic) -> dynamic
{
  return  ((a > 0)) ? 1 : ( ((a < 0)) ? -1 : 0);
}

func gn(x: dynamic) -> dynamic
{
  var sg: dynamic = 1;
  var c: dynamic = cpp_uninitialized();
  while ((((((cpp_assign(c, "=", getchar())) < cpp_char("0")) || (c > cpp_char("9")))) && (c != cpp_char("-"))))
  {
  }
   ((c == cpp_char("-"))) ? (cpp_assign(sg, "=", cpp_assign(x, "=", 0))) : (cpp_assign(x, "=", (c - cpp_char("0"))));
  while ((((cpp_assign(c, "=", getchar())) >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = (((x * 10) + c) - cpp_char("0"));
  }
  x *= sg;
}

func gn(x: dynamic) -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  gn(t);
  x = t;
}

func gn(x: dynamic) -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  gn(t);
  x = t;
}

func gn(x: dynamic) -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  scanf("%lf", (&t));
  x = t;
}

func gn(x: dynamic) -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  scanf("%lf", (&t));
  x = t;
}

func gs(s: dynamic) -> dynamic
{
  scanf("%s", s);
}

func gc(c: dynamic) -> dynamic
{
  while ((((cpp_assign(c, "=", getchar())) > 126) || (c < 33)))
  {
  }
}

func pc(c: dynamic) -> dynamic
{
  putchar(c);
}

func sqr(a: dynamic) -> dynamic
{
  return (a * a);
}

func sqrf(a: dynamic) -> dynamic
{
  return (a * a);
}

var inf: dynamic = 0x3f3f3f3f;

var pi: dynamic = 3.14159265358979323846264338327950288;

var eps: dynamic = 1e-6;

var mo: dynamic = (1e9 + 7);

func qp(a: dynamic, b: dynamic) -> dynamic
{
  var n: dynamic = 1;
  while (true)
  {
    if ((b & 1))
    {
      n = (((1 * n) * a) % mo);
    }
    a = (((1 * a) * a) % mo);
    if (!((cpp_assign(b, ">>=", 1))))
    {
      break;
    }
  }
  return n;
}

var memo: dynamic = cpp_array(10, 777);

var memo2: dynamic = cpp_array(10, 777);

var fac: dynamic = cpp_array(4444);

var ifac: dynamic = cpp_array(4444);

var s: dynamic = cpp_array(777);

var po: dynamic = cpp_array(777, 11);

var invpo: dynamic = cpp_array(777, 11);

var n: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  {
    var d: dynamic = (0);
    var ed: dynamic = (11);
    while ((d < ed))
    {
      po[d][0] = 1;
      {
        var i: dynamic = (1);
        var ed: dynamic = (777);
        while ((i < ed))
        {
          po[d][i] = (((1 * po[d][(i - 1)]) * d) % mo);
          i += 1;
        }
      }
      {
        var i: dynamic = (0);
        var ed: dynamic = (777);
        while ((i < ed))
        {
          invpo[d][i] = qp(po[d][i], (mo - 2));
          i += 1;
        }
      }
      d += 1;
    }
  }
  gs((s + 1));
  n = strlen((s + 1));
  fac[0] = 1;
  {
    var i: dynamic = (1);
    var ed: dynamic = (4444);
    while ((i < ed))
    {
      fac[i] = (((1 * fac[(i - 1)]) * i) % mo);
      i += 1;
    }
  }
  {
    var i: dynamic = (0);
    var ed: dynamic = (4444);
    while ((i < ed))
    {
      ifac[i] = qp(fac[i], (mo - 2));
      i += 1;
    }
  }
  {
    var d: dynamic = (0);
    var ed: dynamic = (10);
    while ((d < ed))
    {
      {
        var a: dynamic = (0);
        var ed: dynamic = ((n + 1));
        while ((a < ed))
        {
          {
            var b: dynamic = 0;
            while (((b + a) <= n))
            {
              ( (((cpp_assign((memo[(a + b)][d]), "=", ((((memo[(a + b)][d]) + ((((((((((((((((1 * ifac[a]) * ifac[b]) % mo) * invpo[d][(a + b)]) % mo) * po[(9 - d)][b]) % mo) * po[10][b]) % mo) * (po[10][a])) % mo) * invpo[9][1]) % mo) * d) % mo)))) % mo))) < 0)) ? cpp_assign((memo[(a + b)][d]), "+=", mo) : (memo[(a + b)][d]));
              ( (((cpp_assign((memo2[(a + b)][d]), "=", ((((memo2[(a + b)][d]) + ((((((((((((((((1 * ifac[a]) * ifac[b]) % mo) * invpo[d][(a + b)]) % mo) * po[(9 - d)][b]) % mo) * po[10][b]) % mo) * (-1)) % mo) * invpo[9][1]) % mo) * d) % mo)))) % mo))) < 0)) ? cpp_assign((memo2[(a + b)][d]), "+=", mo) : (memo2[(a + b)][d]));
              b += 1;
            }
          }
          a += 1;
        }
      }
      d += 1;
    }
  }
  var tot: dynamic = 0;
  {
    var t: dynamic = 1;
    while ((t <= n))
    {
      var nex: dynamic = (s[t] - cpp_char("0"));
      if ((t != n))
      {
        nex -= 1;
      }
      {
        var da: dynamic = 0;
        while ((da <= nex))
        {
          var num: dynamic = [0];
          {
            var j: dynamic = 1;
            while ((j <= (t - 1)))
            {
              num[(s[j] - cpp_char("0"))] += 1;
              j += 1;
            }
          }
          num[da] += 1;
          {
            var d: dynamic = 1;
            while ((d <= 9))
            {
              var L: dynamic = (n - t);
              var p: dynamic = num[d];
              var q: dynamic = 0;
              {
                var tt: dynamic = (d + 1);
                while ((tt <= 9))
                {
                  q += num[tt];
                  tt += 1;
                }
              }
              {
                var ab: dynamic = 0;
                while ((ab <= L))
                {
                  var temp: dynamic = (((((1 * memo[ab][d]) * po[10][p]) + memo2[ab][d])) % mo);
                  ( (((cpp_assign((temp), "=", (((1 * (temp)) * (po[d][L])) % mo))) < 0)) ? cpp_assign((temp), "+=", mo) : (temp));
                  ( (((cpp_assign((temp), "=", (((1 * (temp)) * (fac[L])) % mo))) < 0)) ? cpp_assign((temp), "+=", mo) : (temp));
                  ( (((cpp_assign((temp), "=", (((1 * (temp)) * (ifac[(L - ab)])) % mo))) < 0)) ? cpp_assign((temp), "+=", mo) : (temp));
                  ( (((cpp_assign((temp), "=", (((1 * (temp)) * (po[10][q])) % mo))) < 0)) ? cpp_assign((temp), "+=", mo) : (temp));
                  ( (((cpp_assign((tot), "=", ((((tot) + (temp))) % mo))) < 0)) ? cpp_assign((tot), "+=", mo) : (tot));
                  ab += 1;
                }
              }
              d += 1;
            }
          }
          da += 1;
        }
      }
      t += 1;
    }
  }
  printf("%d\n", tot);
  return 0;
}
