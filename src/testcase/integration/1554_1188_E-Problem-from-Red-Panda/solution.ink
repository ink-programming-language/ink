// Translated from solution.cpp.

func read(res: dynamic)
{
  res = 0;
  var bo = 0;
  var c: dynamic;
  while ((((((cpp_assign(c, "=", getchar())) < cpp_char("0")) || (c > cpp_char("9")))) && (c != cpp_char("-"))))
  {
  }
  if ((c == cpp_char("-")))
  {
    bo = 1;
  } else
  {
    res = (c - 48);
  }
  while ((((cpp_assign(c, "=", getchar())) >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    res = ((((res << 3)) + ((res << 1))) + ((c - 48)));
  }
  if (bo)
  {
    res = ((~res) + 1);
  }
}

var N = (2e6 + 5);

var djq = 998244353;

var k: dynamic;

var n: dynamic;

var a = cpp_array(N);

var cnt = cpp_array(N);

var fac = cpp_array(N);

var inv = cpp_array(N);

var ans: dynamic;

func C(n: dynamic, m: dynamic)
{
  return (((((1 * fac[n]) * inv[m]) % djq) * inv[(n - m)]) % djq);
}

func main()
{
  fac[0] = cpp_assign(inv[0], "=", cpp_assign(inv[1], "=", 1));
  {
    var i = 1;
    while ((i < N))
    {
      fac[i] = (((1 * fac[(i - 1)]) * i) % djq);
      i += 1;
    }
  }
  {
    var i = 2;
    while ((i < N))
    {
      inv[i] = (((1 * ((djq - (djq / i)))) * inv[(djq % i)]) % djq);
      i += 1;
    }
  }
  {
    var i = 2;
    while ((i < N))
    {
      inv[i] = (((1 * inv[i]) * inv[(i - 1)]) % djq);
      i += 1;
    }
  }
  read(k);
  var cur = 0;
  {
    var i = 1;
    while ((i <= k))
    {
      read(a[i]);
      n += a[i];
      i += 1;
    }
  }
  sort((a + 1), ((a + k) + 1));
  {
    var i = 0;
    var j = 1;
    while ((i <= a[k]))
    {
      while ((a[j] < i))
      {
        cnt[(a[cpp_update(j, "++")] % k)] += 1;
      }
      cur += cnt[((((i - 1) + k)) % k)];
      if ((cur > i))
      {
        return cpp_comma(((cout << ans) << endl), 0);
      }
      ans = (((ans + C((((i - cur) + k) - 1), (k - 1)))) % djq);
      if (((((i - cur) + j) - 2) >= (k - 1)))
      {
        ans = ((((ans - C((((i - cur) + j) - 2), (k - 1))) + djq)) % djq);
      }
      i += 1;
    }
  }
  return cpp_comma(((cout << ans) << endl), 0);
}
