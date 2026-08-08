// Translated from solution.cpp.

func read()
{
  var x = 0;
  var f = 1;
  var c = getchar();
  while ((!isdigit(c)))
  {
    if ((c == cpp_char("-")))
    {
      f = -1;
    }
    c = getchar();
  }
  while (isdigit(c))
  {
    x = ((((x << 1)) + ((x << 3))) + ((c ^ 48)));
    c = getchar();
  }
  return if ((f == 1)) x else (-x);
}

var ll = dynamic;

var mod = 998244353;

var inv2 = (((mod + 1)) >> 1);

func fix(x: dynamic)
{
  return (x + ((((x >> 31)) & mod)));
}

func add(x: dynamic, y: dynamic)
{
  return fix(((x + y) - mod));
}

func dec(x: dynamic, y: dynamic)
{
  return fix((x - y));
}

func mul(x: dynamic, y: dynamic)
{
  return ((cpp_cast(x) * y) % mod);
}

func ADD(x: dynamic, y: dynamic)
{
  x = fix(((x + y) - mod));
}

func DEC(x: dynamic, y: dynamic)
{
  x = fix((x - y));
}

func MUL(x: dynamic, y: dynamic)
{
  x = ((cpp_cast(x) * y) % mod);
}

func ksm(x: dynamic, r: dynamic)
{
  var ret = 1;
  {
    var i = 0;
    while ((((1 << i)) <= r))
    {
      if ((((r >> i)) & 1))
      {
        MUL(ret, x);
      }
      MUL(x, x);
      i += 1;
    }
  }
  return ret;
}

func inv(x: dynamic)
{
  return ksm(x, (mod - 2));
}

var N = (2e5 + 4);

var n: dynamic;

var Q: dynamic;

var t: dynamic;

var cnt: dynamic;

var ans: dynamic;

var fac = cpp_array(N);

var ifac = cpp_array(N);

var s = cpp_array(N);

var ss = cpp_array(4);

func prepare()
{
  fac[0] = cpp_assign(fac[1], "=", cpp_assign(ifac[0], "=", cpp_assign(ifac[1], "=", 1)));
  {
    var i = 2;
    while ((i <= n))
    {
      fac[i] = mul(fac[(i - 1)], i);
      i += 1;
    }
  }
  ifac[n] = inv(fac[n]);
  {
    var i = (n - 1);
    while (i)
    {
      ifac[i] = mul(ifac[(i + 1)], (i + 1));
      i -= 1;
    }
  }
}

func C(x: dynamic, y: dynamic)
{
  return if ((((x < y) || (y < 0)))) 0 else mul(fac[x], mul(ifac[y], ifac[(x - y)]));
}

class node
{
  var cnt: dynamic;
  var t: dynamic;
  var ans: dynamic;
  func cntup()
  {
      ans = dec(add(ans, ans), C(cpp_update(cnt, "++"), t));
      if ((!cnt))
      {
        ADD(ans, (t >= 0));
      }
    }
  func cntdw()
  {
      ans = mul(add(ans, C(cpp_update(cnt, "--"), t)), inv2);
      if ((cnt < 0))
      {
        ans = 0;
      }
    }
  func tup()
  {
      ADD(ans, C(cnt, cpp_update(t, "++")));
    }
  func tdw()
  {
      DEC(ans, C(cnt, cpp_update(t, "--")));
    }
}

var a: dynamic;

var b: dynamic;

var c: dynamic;

var d: dynamic;

func tup()
{
  a.tup();
  b.tup();
  t += 1;
}

func tdw()
{
  a.tdw();
  b.tdw();
  t -= 1;
}

func cntup()
{
  a.cntup();
  b.cntup();
  c.cntup();
  d.cntup();
  c.tup();
  d.tup();
  cnt += 1;
}

func cntdw()
{
  a.cntdw();
  b.cntdw();
  c.cntdw();
  d.cntdw();
  c.tdw();
  d.tdw();
  cnt -= 1;
}

func ins(i: dynamic, c: dynamic)
{
  if ((c == cpp_char("b")))
  {
    if ((i & 1))
    {
      tup();
    } else
    {
      tdw();
    }
  } else if ((c == cpp_char("?")))
  {
    cntup();
    if ((i & 1))
    {
      tup();
    }
  }
}

func era(i: dynamic, c: dynamic)
{
  if ((c == cpp_char("b")))
  {
    if ((i & 1))
    {
      tdw();
    } else
    {
      tup();
    }
  } else if ((c == cpp_char("?")))
  {
    cntdw();
    if ((i & 1))
    {
      tdw();
    }
  }
}

func solve()
{
  if ((cnt == 1))
  {
    ans = 0;
    {
      var i = (t & 1);
      while ((i <= cnt))
      {
        ADD(ans, mul(C(cnt, i), abs((i - t))));
        i += 2;
      }
    }
  } else
  {
    ans = add(dec(mul(mul(2, fix(t)), a.ans), mul(mul(2, cnt), b.ans)), dec(mul(cnt, c.ans), mul(fix(t), d.ans)));
  }
  MUL(ans, inv(ksm(2, cnt)));
  write(ans, "\n");
}

func main()
{
  n = read();
  Q = read();
  prepare();
  scanf("%s", (s + 1));
  a.cntdw();
  b.cntdw();
  c.cntdw();
  d.cntdw();
  b.cntdw();
  c.cntdw();
  b.tdw();
  c.tdw();
  {
    var i = 1;
    while ((i <= n))
    {
      ins(i, s[i]);
      i += 1;
    }
  }
  solve();
  {
    var x: dynamic;
    while (cpp_update(Q, "--"))
    {
      x = read();
      scanf("%s", ss);
      era(x, s[x]);
      s[x] = ss[0];
      ins(x, s[x]);
      solve();
    }
  }
  return ((0 - 0));
}
