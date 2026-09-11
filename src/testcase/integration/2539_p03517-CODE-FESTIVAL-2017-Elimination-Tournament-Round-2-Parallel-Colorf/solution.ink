// Translated from solution.cpp.

func FOR(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i = 0; i < (n); i++)");
}

func sz(c: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/");
}

func ten(n: dynamic) -> dynamic
{
  return cpp_expression("#include <bi");
}

func getchar_unlocked(argument_0: dynamic) -> dynamic
{
  return getchar();
}

func putchar_unlocked(c: dynamic) -> dynamic
{
  putchar(c);
}

func mygc(c: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.");
}

func mypc(c: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc");
}

func reader(x: dynamic) -> dynamic
{
  var k: dynamic = cpp_uninitialized();
  var m: dynamic = 0;
  x = 0;
  {
    while (true)
    {
      mygc(k);
      if ((k == cpp_char("-")))
      {
        m = 1;
        break;
      }
      if (((cpp_char("0") <= k) && (k <= cpp_char("9"))))
      {
        x = (k - cpp_char("0"));
        break;
      }
    }
  }
  {
    while (true)
    {
      mygc(k);
      if (((k < cpp_char("0")) || (k > cpp_char("9"))))
      {
        break;
      }
      x = (((x * 10) + k) - cpp_char("0"));
    }
  }
  if (m)
  {
    x = (-x);
  }
}

func reader(x: dynamic) -> dynamic
{
  var k: dynamic = cpp_uninitialized();
  var m: dynamic = 0;
  x = 0;
  {
    while (true)
    {
      mygc(k);
      if ((k == cpp_char("-")))
      {
        m = 1;
        break;
      }
      if (((cpp_char("0") <= k) && (k <= cpp_char("9"))))
      {
        x = (k - cpp_char("0"));
        break;
      }
    }
  }
  {
    while (true)
    {
      mygc(k);
      if (((k < cpp_char("0")) || (k > cpp_char("9"))))
      {
        break;
      }
      x = (((x * 10) + k) - cpp_char("0"));
    }
  }
  if (m)
  {
    x = (-x);
  }
}

func reader(c: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var s: dynamic = 0;
  {
    while (true)
    {
      mygc(i);
      if ((((((i != cpp_char(" ")) && (i != cpp_char("\n"))) && (i != cpp_char("\r"))) && (i != cpp_char("\t"))) && (i != EOF)))
      {
        break;
      }
    }
  }
  c[cpp_update(s, "++")] = i;
  {
    while (true)
    {
      mygc(i);
      if ((((((i == cpp_char(" ")) || (i == cpp_char("\n"))) || (i == cpp_char("\r"))) || (i == cpp_char("\t"))) || (i == EOF)))
      {
        break;
      }
      c[cpp_update(s, "++")] = i;
    }
  }
  c[s] = cpp_char("\u{0}");
  return s;
}

func reader(c: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  {
    while (true)
    {
      mygc(i);
      if ((((((i != cpp_char(" ")) && (i != cpp_char("\n"))) && (i != cpp_char("\r"))) && (i != cpp_char("\t"))) && (i != EOF)))
      {
        break;
      }
    }
  }
  c.push_back(i);
  {
    while (true)
    {
      mygc(i);
      if ((((((i == cpp_char(" ")) || (i == cpp_char("\n"))) || (i == cpp_char("\r"))) || (i == cpp_char("\t"))) || (i == EOF)))
      {
        break;
      }
      c.push_back(i);
    }
  }
  return sz(c);
}

func reader(x: dynamic, y: dynamic) -> dynamic
{
  reader(x);
  reader(y);
}

func reader(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  reader(x);
  reader(y);
  reader(z);
}

func reader(x: dynamic, y: dynamic, z: dynamic, w: dynamic) -> dynamic
{
  reader(x);
  reader(y);
  reader(z);
  reader(w);
}

func writer(x: dynamic, c: dynamic) -> dynamic
{
  var s: dynamic = 0;
  var m: dynamic = 0;
  var f: dynamic = cpp_array(10);
  if ((x < 0))
  {
    m = 1;
    x = (-x);
  }
  while (x)
  {
    f[cpp_update(s, "++")] = (x % 10);
    x /= 10;
  }
  if ((!s))
  {
    f[cpp_update(s, "++")] = 0;
  }
  if (m)
  {
    mypc(cpp_char("-"));
  }
  while (cpp_update(s, "--"))
  {
    mypc((f[s] + cpp_char("0")));
  }
  mypc(c);
}

func writer(x: dynamic, c: dynamic) -> dynamic
{
  var s: dynamic = 0;
  var m: dynamic = 0;
  var f: dynamic = cpp_array(20);
  if ((x < 0))
  {
    m = 1;
    x = (-x);
  }
  while (x)
  {
    f[cpp_update(s, "++")] = (x % 10);
    x /= 10;
  }
  if ((!s))
  {
    f[cpp_update(s, "++")] = 0;
  }
  if (m)
  {
    mypc(cpp_char("-"));
  }
  while (cpp_update(s, "--"))
  {
    mypc((f[s] + cpp_char("0")));
  }
  mypc(c);
}

func writer(c: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((c[i] != cpp_char("\u{0}")))
    {
      mypc(c[i]);
      i += 1;
    }
  }
}

func writer(x: dynamic, c: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((x[i] != cpp_char("\u{0}")))
    {
      mypc(x[i]);
      i += 1;
    }
  }
  mypc(c);
}

func writer(x: dynamic, c: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((x[i] != cpp_char("\u{0}")))
    {
      mypc(x[i]);
      i += 1;
    }
  }
  mypc(c);
}

func writerLn(x: dynamic) -> dynamic
{
  writer(x, cpp_char("\n"));
}

func writerLn(x: dynamic, y: dynamic) -> dynamic
{
  writer(x, cpp_char(" "));
  writer(y, cpp_char("\n"));
}

func writerLn(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  writer(x, cpp_char(" "));
  writer(y, cpp_char(" "));
  writer(z, cpp_char("\n"));
}

func writerArr(x: dynamic, n: dynamic) -> dynamic
{
  if ((!n))
  {
    mypc(cpp_char("\n"));
    return;
  }
  FOR(i, (n - 1));
  writer(x[i], cpp_char(" "));
  writer(x[(n - 1)], cpp_char("\n"));
}

func writerArr(x: dynamic) -> dynamic
{
  writerArr(x.data(), cpp_cast(x.size()));
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
  }
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  (b) ? gcd(b, (a % b)) : a;
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return ((a / gcd(a, b)) * b);
}

func mod_pow(a: dynamic, n: dynamic, mod: dynamic) -> dynamic
{
  var ret: dynamic = 1;
  var p: dynamic = (a % mod);
  while (n)
  {
    if ((n & 1))
    {
      ret = ((ret * p) % mod);
    }
    p = ((p * p) % mod);
    n >>= 1;
  }
  return ret;
}

func extgcd(a: dynamic, b: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  {
    var u: dynamic = cpp_assign(y, "=", 1);
    var v: dynamic = cpp_assign(x, "=", 0);
    while (a)
    {
      var q: dynamic = (b / a);
      swap(cpp_assign(x, "-=", (q * u)), u);
      swap(cpp_assign(y, "-=", (q * v)), v);
      swap(cpp_assign(b, "-=", (q * a)), a);
    }
  }
  return b;
}

func mod_inv(a: dynamic, m: dynamic) -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  extgcd(a, m, x, y);
  return (((m + (x % m))) % m);
}

class UnionFind
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  func UnionFind(n: dynamic) -> dynamic
  {
      self->n = cpp_construct(n);
      self->a = cpp_construct(n, -1);
    }
  func find(x: dynamic) -> dynamic
  {
      return  ((a[x] < 0)) ? x : (cpp_assign(a[x], "=", find(a[x])));
    }
  func same(x: dynamic, y: dynamic) -> dynamic
  {
      return (find(x) == find(y));
    }
  func same(p: dynamic) -> dynamic
  {
      return same(p.first, p.second);
    }
  func unite(x: dynamic, y: dynamic) -> dynamic
  {
      x = find(x);
      y = find(y);
      if ((x == y))
      {
        return false;
      }
      if ((a[x] > a[y]))
      {
        swap(x, y);
      }
      a[x] += a[y];
      a[y] = x;
      n -= 1;
      return true;
    }
  func unite(p: dynamic) -> dynamic
  {
      return unite(p.first, p.second);
    }
  func size() -> dynamic
  {
      return n;
    }
  func size(x: dynamic) -> dynamic
  {
      return (-a[find(x)]);
    }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  reader(n, m, k);
  FOR(i, n);
  reader(c[i]);
  var vp: dynamic = cpp_uninitialized();
  var uf: dynamic = cpp_construct((n + k));
  var free: dynamic = 0;
  var ans: dynamic = 0;
  sort(vp.begin(), vp.end());
  for (var wab: dynamic in vp)
  {
    if ((uf.size() <= (free + 1)))
    {
      break;
    }
    var w: dynamic = cpp_uninitialized();
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    tie(w, a, b) = wab;
    if (uf.unite(a, b))
    {
      ans += w;
    }
  }
  if ((uf.size() > (free + 1)))
  {
    ans = -1;
  }
  writerLn(ans);
  return 0;
}

func FOR(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var w: dynamic = cpp_uninitialized();
    reader(a, b, w);
    a -= 1;
    b -= 1;
    vp.emplace_back(w, a, b);
  }

func FOR(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    c[i] -= 1;
    if ((c[i] == -1))
    {
      free += 1;
    } else
    {
      uf.unite(i, (n + c[i]));
    }
  }
