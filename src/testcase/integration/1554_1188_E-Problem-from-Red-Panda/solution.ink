// Translated from solution.cpp.

func read(res: dynamic) -> dynamic
{
  res = 0;
  var bo: dynamic = 0;
  var c: dynamic = cpp_uninitialized();
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

var N: dynamic = (2e6 + 5);

var djq: dynamic = 998244353;

var k: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var cnt: dynamic = cpp_array(N);

var fac: dynamic = cpp_array(N);

var inv: dynamic = cpp_array(N);

var ans: dynamic = cpp_uninitialized();

func C(n: dynamic, m: dynamic) -> dynamic
{
  return (((((1 * fac[n]) * inv[m]) % djq) * inv[(n - m)]) % djq);
}

func main() -> dynamic
{
  fac[0] = cpp_assign(inv[0], "=", cpp_assign(inv[1], "=", 1));
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      fac[i] = (((1 * fac[(i - 1)]) * i) % djq);
      i += 1;
    }
  }
  {
    var i: dynamic = 2;
    while ((i < N))
    {
      inv[i] = (((1 * ((djq - (djq / i)))) * inv[(djq % i)]) % djq);
      i += 1;
    }
  }
  {
    var i: dynamic = 2;
    while ((i < N))
    {
      inv[i] = (((1 * inv[i]) * inv[(i - 1)]) % djq);
      i += 1;
    }
  }
  read(k);
  var cur: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      read(a[i]);
      n += a[i];
      i += 1;
    }
  }
  sort((a + 1), ((a + k) + 1));
  {
    var i: dynamic = 0;
    var j: dynamic = 1;
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
