// Translated from solution.cpp.

func bigmod(p: dynamic, e: dynamic, M: dynamic) -> dynamic
{
  var ret: dynamic = 1;
  {
    while ((e > 0))
    {
      if ((e & 1))
      {
        ret = (((ret * p)) % M);
      }
      p = (((p * p)) % M);
      e >>= 1;
    }
  }
  return cpp_cast(ret);
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func modinverse(a: dynamic, M: dynamic) -> dynamic
{
  return bigmod(a, (M - 2), M);
}

func bpow(p: dynamic, e: dynamic) -> dynamic
{
  var ret: dynamic = 1;
  {
    while ((e > 0))
    {
      if ((e & 1))
      {
        ret = ((ret * p));
      }
      p = ((p * p));
      e >>= 1;
    }
  }
  return cpp_cast(ret);
}

func toInt(s: dynamic) -> dynamic
{
  var sm: dynamic = cpp_uninitialized();
  (ss >> sm);
  return sm;
}

func toLlint(s: dynamic) -> dynamic
{
  var sm: dynamic = cpp_uninitialized();
  (ss >> sm);
  return sm;
}

var cs: dynamic = cpp_uninitialized();

var ln: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(1009, 1009, 2, 2);

var p0: dynamic = cpp_array(1009, 1009);

var bl0: dynamic = cpp_array(1009, 1009);

var bl: dynamic = cpp_array(1009, 1009, 2, 2);

var a: dynamic = cpp_array(1009);

var b: dynamic = cpp_array(1009);

func dppr(i: dynamic, k: dynamic) -> dynamic
{
  if ((i == -1))
  {
    if ((k == 1005))
    {
      return 1;
    }
    return 0;
  }
  var pr: dynamic = p0[i][k];
  if ((bl0[i][k] == 1))
  {
    return pr;
  }
  bl0[i][k] = 1;
  pr = 0;
  var qk: dynamic = cpp_uninitialized();
  {
    var j: dynamic = 0;
    while ((j <= 9))
    {
      qk = k;
      if (((k != 1005) && (((j == 4) || (j == 7)))))
      {
        if ((k == 1004))
        {
          qk = i;
        } else if (((qk - i) <= q))
        {
          qk = 1005;
        } else
        {
          qk = i;
        }
      }
      pr += dppr((i - 1), qk);
      if ((pr >= 1000000007))
      {
        pr -= 1000000007;
      }
      j += 1;
    }
  }
  return pr;
}

func qry(bg: dynamic, sm: dynamic, i: dynamic, k: dynamic) -> dynamic
{
  if ((i == -1))
  {
    if ((k == 1005))
    {
      return 1;
    }
    return 0;
  }
  var pr: dynamic = p[bg][sm][i][k];
  if ((bl[bg][sm][i][k] == cs))
  {
    return pr;
  }
  bl[bg][sm][i][k] = cs;
  if (((bg == 1) && (sm == 1)))
  {
    return cpp_assign(pr, "=", dppr(i, k));
  }
  pr = 0;
  var qbg: dynamic = cpp_uninitialized();
  var qsm: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var qk: dynamic = cpp_uninitialized();
  l =  (((bg == 1))) ? 0 : ((a[i] - cpp_char("0")));
  r =  (((sm == 1))) ? 9 : ((b[i] - cpp_char("0")));
  {
    var j: dynamic = l;
    while ((j <= r))
    {
      qbg =  (((bg == 1))) ? 1 : ((j > l));
      qsm =  (((sm == 1))) ? 1 : ((j < r));
      qk = k;
      if (((k != 1005) && (((j == 4) || (j == 7)))))
      {
        if ((k == 1004))
        {
          qk = i;
        } else if (((qk - i) <= q))
        {
          qk = 1005;
        } else
        {
          qk = i;
        }
      }
      pr += qry(qbg, qsm, (i - 1), qk);
      if ((pr >= 1000000007))
      {
        pr -= 1000000007;
      }
      j += 1;
    }
  }
  return pr;
}

func main() -> dynamic
{
  cs = 1;
  var t: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  scanf("%d%d", (&t), (&q));
  while (cpp_update(t, "--"))
  {
    scanf(" %s %s", a, b);
    j = strlen(a);
    ln = strlen(b);
    reverse((&a[0]), (&a[j]));
    reverse((&b[0]), (&b[ln]));
    {
      i = j;
      while ((i < 1003))
      {
        a[i] = cpp_char("0");
        i += 1;
      }
    }
    {
      i = ln;
      while ((i < 1003))
      {
        b[i] = cpp_char("0");
        i += 1;
      }
    }
    printf("%I64d\n", qry(0, 0, 1002, 1004));
    cs += 1;
  }
  return 0;
}
