// Translated from solution.cpp.

var maxn = (3e5 + 100);

var a = cpp_array(maxn);

var b = cpp_array(maxn);

var res: dynamic;

var sumaL = cpp_array(maxn);

var sumaR = cpp_array(maxn);

var sumbL = cpp_array(maxn);

var sumbR = cpp_array(maxn);

var sal = cpp_array(maxn);

var sar = cpp_array(maxn);

var sbl = cpp_array(maxn);

var sbr = cpp_array(maxn);

func downToRight(ans: dynamic, i: dynamic, n: dynamic, t: dynamic)
{
  ans += ((((((t * sumbR[(i + 1)]) + sbr[(i + 1)]) + ((((t + n) - i)) * sumaR[i])) - sal[(i - 1)]) + sal[n]) - (sumaL[(i - 1)] * (((n - i) + 1))));
  res = max(res, ans);
}

func upToRight(ans: dynamic, i: dynamic, n: dynamic, t: dynamic)
{
  ans += ((((((t * sumaR[(i + 1)]) + sar[(i + 1)]) + ((((t + n) - i)) * sumbR[i])) - sbl[(i - 1)]) + sbl[n]) - (sumbL[(i - 1)] * (((n - i) + 1))));
  res = max(res, ans);
}

func main()
{
  var n: dynamic;
  var ca = 0;
  while ((scanf("%d", (&n)) != EOF))
  {
    if (cpp_update(ca, "++"))
    {
      sumaL[0] = cpp_assign(sumaL[(n + 1)], "=", 0);
      sumbL[0] = cpp_assign(sumbL[(n + 1)], "=", 0);
      sumaR[0] = cpp_assign(sumaR[(n + 1)], "=", 0);
      sumbL[0] = cpp_assign(sumbR[(n + 1)], "=", 0);
      sal[0] = cpp_assign(sal[(n + 1)], "=", 0);
      sar[0] = cpp_assign(sar[(n + 1)], "=", 0);
      sbl[0] = cpp_assign(sbl[(n + 1)], "=", 0);
      sbr[0] = cpp_assign(sbr[(n + 1)], "=", 0);
    }
    {
      var i = (1);
      while ((i <= (n)))
      {
        scanf("%lld", (a + i));
        sumaL[i] = (sumaL[(i - 1)] + a[i]);
        i += 1;
      }
    }
    {
      var i = (1);
      while ((i <= (n)))
      {
        scanf("%lld", (b + i));
        sumbL[i] = (sumbL[(i - 1)] + b[i]);
        i += 1;
      }
    }
    if ((n == 1))
    {
      printf("%lld\n", b[1]);
      continue;
    }
    {
      var i = (n);
      while ((i >= (1)))
      {
        sumaR[i] = (sumaR[(i + 1)] + a[i]);
        sumbR[i] = (sumbR[(i + 1)] + b[i]);
        i -= 1;
      }
    }
    {
      var i = (1);
      while ((i <= (n)))
      {
        sal[i] = (sal[(i - 1)] + sumaL[i]);
        sbl[i] = (sbl[(i - 1)] + sumbL[i]);
        i += 1;
      }
    }
    {
      var i = (n);
      while ((i >= (0)))
      {
        sar[i] = (sar[(i + 1)] + sumaR[i]);
        sbr[i] = (sbr[(i + 1)] + sumbR[i]);
        i -= 1;
      }
    }
    res = 0;
    res = ((sar[2] + sbl[n]) + (sumbL[n] * ((n - 1))));
    var t = 0;
    var ans = 0;
    {
      var i = (1);
      while ((i <= (n)))
      {
        t += 1;
        if ((i & 1))
        {
          ans += (t * b[i]);
        } else
        {
          ans += (t * a[i]);
        }
        if ((i == n))
        {
          res = max(res, ans);
          i += 1;
          continue;
        }
        t += 1;
        if ((i & 1))
        {
          ans += (t * b[(i + 1)]);
          if ((i < (n - 1)))
          {
            downToRight(ans, (i + 1), n, t);
          }
        } else
        {
          ans += (t * a[(i + 1)]);
          if ((i < (n - 1)))
          {
            upToRight(ans, (i + 1), n, t);
          }
        }
        i += 1;
      }
    }
    printf("%lld\n", res);
  }
  return 0;
}
