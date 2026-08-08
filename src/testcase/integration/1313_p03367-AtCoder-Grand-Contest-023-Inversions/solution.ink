// Translated from solution.cpp.

var ld = cpp_expression("#inclu");

var ull = dynamic;

var ll = dynamic;

var pii = cpp_expression("#include<bits/");

var iiii = cpp_expression("#include<bits/");

var mp = cpp_expression("#include<");

var INF = cpp_expression("#include<b");

var MOD = cpp_expression("#include<b");

func rep(i: dynamic, x: dynamic)
{
  cpp_macro("for(int (i)=0;(i)<(x);(i)++)");
}

func getint()
{
  var x = 0;
  var p = 1;
  var c = getchar();
  while ((c <= 32))
  {
    c = getchar();
  }
  if ((c == 45))
  {
    p = (-p);
    c = getchar();
  }
  while ((c > 32))
  {
    x = (((x * 10) + c) - 48);
    c = getchar();
  }
  return (x * p);
}

var N = (2e5 + 5);

var inv2 = (((MOD + 1)) / 2);

var n: dynamic;

var a = cpp_array(N);

var b = cpp_array(N);

var cnt = cpp_array(N);

var ib = cpp_array(N);

var pre = cpp_array(N);

var dx = cpp_array(N);

var dy = cpp_array(N);

func add(x: dynamic, y: dynamic)
{
  x += y;
  if ((x >= MOD))
  {
    x -= MOD;
  }
}

func sub(x: dynamic, y: dynamic)
{
  x -= y;
  if ((x < 0))
  {
    x += MOD;
  }
}

func sub2(x: dynamic, y: dynamic)
{
  x -= y;
  return if (((x < 0))) (x + MOD) else x;
}

func mul(x: dynamic, y: dynamic)
{
  var ans = ((1 * x) * y);
  return (ans % MOD);
}

func modpow(x: dynamic, y: dynamic)
{
  var ans = 1;
  while (y)
  {
    if ((y & 1))
    {
      ans = mul(ans, x);
    }
    x = mul(x, x);
    y >>= 1;
  }
  return ans;
}

func modinv(x: dynamic)
{
  return modpow(x, (MOD - 2));
}

func upd(x: dynamic, y: dynamic)
{
  while ((x < N))
  {
    add(dy[x], y);
    dx[x] += 1;
    x += (x & (-x));
  }
}

func qry(d: dynamic, x: dynamic)
{
  var ans = 0;
  while (x)
  {
    add(ans, d[x]);
    x -= (x & (-x));
  }
  return ans;
}

func main()
{
  n = getint();
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] = getint();
      cnt[a[i]] += 1;
      i += 1;
    }
  }
  {
    var i = (n - 1);
    while ((i >= 1))
    {
      cnt[i] += cnt[(i + 1)];
      i -= 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      cnt[i] -= (n - i);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((cnt[i] <= 0))
      {
        return (!printf("%d\n", 0));
      }
      i += 1;
    }
  }
  var s = 1;
  pre[0] = cpp_assign(b[0], "=", 1);
  {
    var i = 1;
    while ((i <= n))
    {
      s = mul(s, cnt[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      b[i] = mul((cnt[i] - 1), modinv(cnt[i]));
      if ((!b[i]))
      {
        pre[i] = i;
        b[i] = b[(i - 1)];
      } else
      {
        pre[i] = pre[(i - 1)];
        b[i] = mul(b[(i - 1)], b[i]);
      }
      ib[i] = modinv(b[i]);
      i += 1;
    }
  }
  var res = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      add(res, mul(b[a[i]], mul(s, mul(inv2, sub2(qry(dy, a[i]), qry(dy, (pre[a[i]] - 1)))))));
      upd(a[i], ib[a[i]]);
      i += 1;
    }
  }
  memset(dx, 0, cpp_sizeof((dx)));
  memset(dy, 0, cpp_sizeof((dy)));
  {
    var i = n;
    while ((i >= 1))
    {
      sub(res, mul(b[a[i]], mul(s, mul(inv2, sub2(qry(dy, (a[i] - 1)), qry(dy, (pre[a[i]] - 1)))))));
      add(res, mul(s, qry(dx, (a[i] - 1))));
      upd(a[i], ib[a[i]]);
      i -= 1;
    }
  }
  write(res, "\n");
  return 0;
}
