// Translated from solution.cpp.

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var c: dynamic = getchar();
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
  return  ((f == 1)) ? x : (-x);
}

var ll: dynamic = dynamic;

var mod: dynamic = 998244353;

var inv2: dynamic = (((mod + 1)) >> 1);

func fix(x: dynamic) -> dynamic
{
  return (x + ((((x >> 31)) & mod)));
}

func add(x: dynamic, y: dynamic) -> dynamic
{
  return fix(((x + y) - mod));
}

func dec(x: dynamic, y: dynamic) -> dynamic
{
  return fix((x - y));
}

func mul(x: dynamic, y: dynamic) -> dynamic
{
  return ((cpp_cast(x) * y) % mod);
}

func ADD(x: dynamic, y: dynamic) -> dynamic
{
  x = fix(((x + y) - mod));
}

func DEC(x: dynamic, y: dynamic) -> dynamic
{
  x = fix((x - y));
}

func MUL(x: dynamic, y: dynamic) -> dynamic
{
  x = ((cpp_cast(x) * y) % mod);
}

func ksm(x: dynamic, r: dynamic) -> dynamic
{
  var ret: dynamic = 1;
  {
    var i: dynamic = 0;
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

func inv(x: dynamic) -> dynamic
{
  return ksm(x, (mod - 2));
}

var N: dynamic = (2e5 + 4);

var n: dynamic = cpp_uninitialized();

var Q: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var fac: dynamic = cpp_array(N);

var ifac: dynamic = cpp_array(N);

var s: dynamic = cpp_array(N);

var ss: dynamic = cpp_array(4);

func prepare() -> dynamic
{
  fac[0] = cpp_assign(fac[1], "=", cpp_assign(ifac[0], "=", cpp_assign(ifac[1], "=", 1)));
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      fac[i] = mul(fac[(i - 1)], i);
      i += 1;
    }
  }
  ifac[n] = inv(fac[n]);
  {
    var i: dynamic = (n - 1);
    while (i)
    {
      ifac[i] = mul(ifac[(i + 1)], (i + 1));
      i -= 1;
    }
  }
}

func C(x: dynamic, y: dynamic) -> dynamic
{
  return  ((((x < y) || (y < 0)))) ? 0 : mul(fac[x], mul(ifac[y], ifac[(x - y)]));
}

class node
{
  var cnt: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  func cntup() -> dynamic
  {
      ans = dec(add(ans, ans), C(cpp_update(cnt, "++"), t));
      if ((!cnt))
      {
        ADD(ans, (t >= 0));
      }
    }
  func cntdw() -> dynamic
  {
      ans = mul(add(ans, C(cpp_update(cnt, "--"), t)), inv2);
      if ((cnt < 0))
      {
        ans = 0;
      }
    }
  func tup() -> dynamic
  {
      ADD(ans, C(cnt, cpp_update(t, "++")));
    }
  func tdw() -> dynamic
  {
      DEC(ans, C(cnt, cpp_update(t, "--")));
    }
}

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

func tup() -> dynamic
{
  a.tup();
  b.tup();
  t += 1;
}

func tdw() -> dynamic
{
  a.tdw();
  b.tdw();
  t -= 1;
}

func cntup() -> dynamic
{
  a.cntup();
  b.cntup();
  c.cntup();
  d.cntup();
  c.tup();
  d.tup();
  cnt += 1;
}

func cntdw() -> dynamic
{
  a.cntdw();
  b.cntdw();
  c.cntdw();
  d.cntdw();
  c.tdw();
  d.tdw();
  cnt -= 1;
}

func ins(i: dynamic, c: dynamic) -> dynamic
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

func era(i: dynamic, c: dynamic) -> dynamic
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

func solve() -> dynamic
{
  if ((cnt == 1))
  {
    ans = 0;
    {
      var i: dynamic = (t & 1);
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

func main() -> dynamic
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      ins(i, s[i]);
      i += 1;
    }
  }
  solve();
  {
    var x: dynamic = cpp_uninitialized();
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
