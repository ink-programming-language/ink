// Translated from solution.cpp.

var maxn = (int_cpp(2e5) + 100);

var mod = (int_cpp(1e9) + 7);

class binary
{
  var n: dynamic;
  var a: dynamic = cpp_array(maxn);
  func read()
  {
      var ch: dynamic;
      while ((((cpp_assign(ch, "=", getchar())) != cpp_char("0")) && (ch != cpp_char("1"))))
      {
      }
      a[cpp_assign(n, "=", 1)] = (ch - cpp_char("0"));
      while ((((cpp_assign(ch, "=", getchar())) == cpp_char("0")) || (ch == cpp_char("1"))))
      {
        a[cpp_update(n, "++")] = (ch - cpp_char("0"));
      }
      {
        var i = 1;
        while ((i <= (n / 2)))
        {
          swap(a[i], a[((n - i) + 1)]);
          i += 1;
        }
      }
    }
  func dec()
  {
      a[1] -= 1;
      {
        var i = 1;
        while ((i <= n))
        {
          if ((a[i] >= 0))
          {
            break;
          } else
          {
            a[i] += 2;
            a[(i + 1)] -= 1;
          }
          i += 1;
        }
      }
      while (((n > 0) && (a[n] == 0)))
      {
        n -= 1;
      }
    }
}

var A: dynamic;

var B: dynamic;

var fact = cpp_array(maxn);

var nfact = cpp_array(maxn);

var origin = cpp_array(maxn);

var cnt = cpp_array(maxn);

func getC(n: dynamic, m: dynamic)
{
  if ((n == -1))
  {
    return ((m == 0));
  }
  return ((((fact[n] * nfact[m]) % mod) * nfact[(n - m)]) % mod);
}

func calc(flag: dynamic)
{
  if (flag)
  {
    if ((((cnt[2] - cnt[1]) != 0) && ((cnt[2] - cnt[1]) != 1)))
    {
      return 0;
    }
    return ((getC(((cnt[2] - 1) + cnt[0]), cnt[0]) * getC((((cnt[1] + 1) - 1) + cnt[3]), cnt[3])) % mod);
  } else
  {
    if ((((cnt[2] - cnt[1]) != 0) && ((cnt[1] - cnt[2]) != 1)))
    {
      return 0;
    }
    return ((getC((((cnt[2] + 1) - 1) + cnt[0]), cnt[0]) * getC(((cnt[1] - 1) + cnt[3]), cnt[3])) % mod);
  }
}

func work(num: dynamic)
{
  {
    var i = 0;
    while ((i < 4))
    {
      cnt[i] = origin[i];
      i += 1;
    }
  }
  var ans = 0;
  var i: dynamic;
  {
    i = (num.n - 1);
    while ((i > 0))
    {
      if ((num.a[i] == 1))
      {
        if ((cnt[(num.a[(i + 1)] * 2)] > 0))
        {
          cnt[(num.a[(i + 1)] * 2)] -= 1;
          ans = (((ans + calc(0))) % mod);
          cnt[(num.a[(i + 1)] * 2)] += 1;
        }
      }
      if (((cpp_update(cnt[((num.a[(i + 1)] * 2) + num.a[i])], "--")) < 0))
      {
        break;
      }
      i -= 1;
    }
  }
  if ((i == 0))
  {
    ans = (((ans + 1)) % mod);
  }
  return ans;
}

func POW(num: dynamic, times: dynamic)
{
  var ans = 1;
  while (times)
  {
    if ((times & 1))
    {
      ans = ((ans * num) % mod);
    }
    num = ((num * num) % mod);
    times >>= 1;
  }
  return ans;
}

func solve()
{
  var n = 1;
  {
    var i = 0;
    while ((i < 4))
    {
      n += origin[i];
      i += 1;
    }
  }
  if (((n > B.n) || (A.n > n)))
  {
    puts("0");
    return;
  }
  fact[0] = cpp_assign(nfact[0], "=", 1);
  {
    var i = 1;
    while ((i <= n))
    {
      fact[i] = ((fact[(i - 1)] * i) % mod);
      nfact[i] = POW(fact[i], (mod - 2));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 4))
    {
      cnt[i] = origin[i];
      i += 1;
    }
  }
  var sA = 0;
  var sB = calc(1);
  if ((A.n == n))
  {
    sA = work(A);
  }
  if ((B.n == n))
  {
    sB = work(B);
  }
  printf("%d\n", ((((sB - sA) + mod)) % mod));
}

func main()
{
  A.read();
  A.dec();
  B.read();
  {
    var i = 0;
    while ((i < 4))
    {
      scanf("%d", (&origin[i]));
      i += 1;
    }
  }
  solve();
  return 0;
}
