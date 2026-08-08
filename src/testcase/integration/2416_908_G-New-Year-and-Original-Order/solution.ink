// Translated from solution.cpp.

func getre()
{
  var x = 0;
  printf("%d\n", (1 / x));
}

func gettle()
{
  var res = 1;
  while (1)
  {
    res <<= 1;
  }
  printf("%d\n", res);
}

func upmin(a: dynamic, b: dynamic)
{
  return if ((a > b)) cpp_comma(cpp_assign(a, "=", b), 1) else 0;
}

func upmax(a: dynamic, b: dynamic)
{
  return if ((a < b)) cpp_comma(cpp_assign(a, "=", b), 1) else 0;
}

func flo(a: dynamic, b: dynamic)
{
  return if ((a >= 0)) (a / b) else ((-(((((-a) - 1)) / b))) - 1);
}

func cei(a: dynamic, b: dynamic)
{
  return if ((a > 0)) ((((a - 1)) / b) + 1) else (-(((-a) / b)));
}

func gcd(a: dynamic, b: dynamic)
{
  return if (b) gcd(b, (a % b)) else a;
}

func sgn(a: dynamic)
{
  return if ((a > 0)) 1 else (if ((a < 0)) -1 else 0);
}

func gn(x: dynamic)
{
  var sg = 1;
  var c: dynamic;
  while ((((((cpp_assign(c, "=", getchar())) < cpp_char("0")) || (c > cpp_char("9")))) && (c != cpp_char("-"))))
  {
  }
  if ((c == cpp_char("-"))) (cpp_assign(sg, "=", cpp_assign(x, "=", 0))) else (cpp_assign(x, "=", (c - cpp_char("0"))));
  while ((((cpp_assign(c, "=", getchar())) >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = (((x * 10) + c) - cpp_char("0"));
  }
  x *= sg;
}

func gn(x: dynamic)
{
  var t: dynamic;
  gn(t);
  x = t;
}

func gn(x: dynamic)
{
  var t: dynamic;
  gn(t);
  x = t;
}

func gn(x: dynamic)
{
  var t: dynamic;
  scanf("%lf", (&t));
  x = t;
}

func gn(x: dynamic)
{
  var t: dynamic;
  scanf("%lf", (&t));
  x = t;
}

func gs(s: dynamic)
{
  scanf("%s", s);
}

func gc(c: dynamic)
{
  while ((((cpp_assign(c, "=", getchar())) > 126) || (c < 33)))
  {
  }
}

func pc(c: dynamic)
{
  putchar(c);
}

func sqr(a: dynamic)
{
  return (a * a);
}

func sqrf(a: dynamic)
{
  return (a * a);
}

var inf = 0x3f3f3f3f;

var pi = 3.14159265358979323846264338327950288;

var eps = 1e-6;

var mo = (1e9 + 7);

func qp(a: dynamic, b: dynamic)
{
  var n = 1;
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

var memo = cpp_array(10, 777);

var memo2 = cpp_array(10, 777);

var fac = cpp_array(4444);

var ifac = cpp_array(4444);

var s = cpp_array(777);

var po = cpp_array(777, 11);

var invpo = cpp_array(777, 11);

var n: dynamic;

func main()
{
  {
    var d = (0);
    var ed = (11);
    while ((d < ed))
    {
      po[d][0] = 1;
      {
        var i = (1);
        var ed = (777);
        while ((i < ed))
        {
          po[d][i] = (((1 * po[d][(i - 1)]) * d) % mo);
          i += 1;
        }
      }
      {
        var i = (0);
        var ed = (777);
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
    var i = (1);
    var ed = (4444);
    while ((i < ed))
    {
      fac[i] = (((1 * fac[(i - 1)]) * i) % mo);
      i += 1;
    }
  }
  {
    var i = (0);
    var ed = (4444);
    while ((i < ed))
    {
      ifac[i] = qp(fac[i], (mo - 2));
      i += 1;
    }
  }
  {
    var d = (0);
    var ed = (10);
    while ((d < ed))
    {
      {
        var a = (0);
        var ed = ((n + 1));
        while ((a < ed))
        {
          {
            var b = 0;
            while (((b + a) <= n))
            {
              (if (((cpp_assign((memo[(a + b)][d]), "=", ((((memo[(a + b)][d]) + ((((((((((((((((1 * ifac[a]) * ifac[b]) % mo) * invpo[d][(a + b)]) % mo) * po[(9 - d)][b]) % mo) * po[10][b]) % mo) * (po[10][a])) % mo) * invpo[9][1]) % mo) * d) % mo)))) % mo))) < 0)) cpp_assign((memo[(a + b)][d]), "+=", mo) else (memo[(a + b)][d]));
              (if (((cpp_assign((memo2[(a + b)][d]), "=", ((((memo2[(a + b)][d]) + ((((((((((((((((1 * ifac[a]) * ifac[b]) % mo) * invpo[d][(a + b)]) % mo) * po[(9 - d)][b]) % mo) * po[10][b]) % mo) * (-1)) % mo) * invpo[9][1]) % mo) * d) % mo)))) % mo))) < 0)) cpp_assign((memo2[(a + b)][d]), "+=", mo) else (memo2[(a + b)][d]));
              b += 1;
            }
          }
          a += 1;
        }
      }
      d += 1;
    }
  }
  var tot = 0;
  {
    var t = 1;
    while ((t <= n))
    {
      var nex = (s[t] - cpp_char("0"));
      if ((t != n))
      {
        nex -= 1;
      }
      {
        var da = 0;
        while ((da <= nex))
        {
          var num = [0];
          {
            var j = 1;
            while ((j <= (t - 1)))
            {
              num[(s[j] - cpp_char("0"))] += 1;
              j += 1;
            }
          }
          num[da] += 1;
          {
            var d = 1;
            while ((d <= 9))
            {
              var L = (n - t);
              var p = num[d];
              var q = 0;
              {
                var tt = (d + 1);
                while ((tt <= 9))
                {
                  q += num[tt];
                  tt += 1;
                }
              }
              {
                var ab = 0;
                while ((ab <= L))
                {
                  var temp = (((((1 * memo[ab][d]) * po[10][p]) + memo2[ab][d])) % mo);
                  (if (((cpp_assign((temp), "=", (((1 * (temp)) * (po[d][L])) % mo))) < 0)) cpp_assign((temp), "+=", mo) else (temp));
                  (if (((cpp_assign((temp), "=", (((1 * (temp)) * (fac[L])) % mo))) < 0)) cpp_assign((temp), "+=", mo) else (temp));
                  (if (((cpp_assign((temp), "=", (((1 * (temp)) * (ifac[(L - ab)])) % mo))) < 0)) cpp_assign((temp), "+=", mo) else (temp));
                  (if (((cpp_assign((temp), "=", (((1 * (temp)) * (po[10][q])) % mo))) < 0)) cpp_assign((temp), "+=", mo) else (temp));
                  (if (((cpp_assign((tot), "=", ((((tot) + (temp))) % mo))) < 0)) cpp_assign((tot), "+=", mo) else (tot));
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
