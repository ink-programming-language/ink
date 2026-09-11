// Translated from solution.cpp.

var md: dynamic = 1000000007;

var maxn: dynamic = 101010;

var inf: dynamic = 2020202020202020202;

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn);

var pow2: dynamic = cpp_array(maxn);

var ans: dynamic = 1;

var fact: dynamic = cpp_array(maxn);

var revfact: dynamic = cpp_array(maxn);

var lol: dynamic = 1;

func upd_ans(t: dynamic) -> dynamic
{
  ans = (((ans * t)) % md);
}

func binpow(a: dynamic, n: dynamic) -> dynamic
{
  if ((n == 0))
  {
    return 1;
  }
  if (((n % 2) == 1))
  {
    return (((binpow(a, (n - 1)) * a)) % md);
  } else
  {
    var b: dynamic = binpow(a, (n / 2));
    return (((b * b)) % md);
  }
}

func do_fact() -> dynamic
{
  fact[0] = 1;
  revfact[0] = 1;
  {
    var i: dynamic = 1;
    while ((i < maxn))
    {
      fact[i] = (((i * fact[(i - 1)])) % md);
      revfact[i] = binpow(fact[i], (md - 2));
      i += 1;
    }
  }
}

func do_c(k: dynamic, n: dynamic) -> dynamic
{
  if ((k < 0))
  {
    return 0;
  }
  if ((k > n))
  {
    return 0;
  }
  var r: dynamic = 1;
  r = (r * fact[n]);
  r = (((r * revfact[k])) % md);
  r = (((r * revfact[(n - k)])) % md);
  return r;
}

func doit(l: dynamic, r: dynamic, it1: dynamic, it2: dynamic, low: dynamic) -> dynamic
{
  if ((l > r))
  {
    return 1;
  }
  if ((it1 > it2))
  {
    return pow2[(r - l)];
  }
  if ((it1 <= it2))
  {
    var s1: dynamic = 0;
    var s2: dynamic = 0;
    if ((a[it1] >= a[it2]))
    {
      if ((a[it1] > low))
      {
        lol = 0;
      } else
      {
        low = a[it1];
        s1 = (do_c((it1 - l), (((r - l) - a[it1]) + 1)));
        var l0: dynamic = (it1 + 1);
        var r0: dynamic = ((it1 + a[it1]) - 1);
        var it10: dynamic = (it1 + 1);
        while ((((!a[it10])) && ((it10 < n))))
        {
          it10 += 1;
        }
        s1 = (((s1 * doit(l0, r0, it10, it2, low))) % md);
      }
    }
    if ((a[it1] <= a[it2]))
    {
      if ((a[it2] > low))
      {
        lol = 0;
      } else
      {
        low = a[it2];
        s2 = do_c((r - it2), (((r - l) - a[it2]) + 1));
        var r0: dynamic = (it2 - 1);
        var l0: dynamic = ((it2 - a[it2]) + 1);
        var it20: dynamic = (it2 - 1);
        while ((((!a[it20])) && ((it20 >= 0))))
        {
          it20 -= 1;
        }
        s2 = (((s2 * doit(l0, r0, it1, it20, low))) % md);
      }
    }
    if ((((a[it1] != 1)) || ((a[it2] != 1))))
    {
      s1 = (((s1 + s2)) % md);
    }
    return s1;
  }
}

func main() -> dynamic
{
  read(n);
  do_fact();
  var it1: dynamic = md;
  var it2: dynamic = 0;
  pow2[0] = 1;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      pow2[(i + 1)] = (((pow2[i] << 1)) % md);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      if (a[i])
      {
        it2 = i;
      }
      if ((a[i] && ((it1 == md))))
      {
        it1 = i;
      }
      i += 1;
    }
  }
  ans = doit(0, (n - 1), it1, it2, md);
  ans = (ans * lol);
  write(ans);
  read(n);
  return 0;
}
