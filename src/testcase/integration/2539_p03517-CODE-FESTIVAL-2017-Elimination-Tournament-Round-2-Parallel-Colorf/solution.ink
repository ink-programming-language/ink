// Translated from solution.cpp.

func FOR(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = 0; i < (n); i++)");
}

func sz(c: dynamic)
{
  return cpp_expression("#include <bits/");
}

func ten(n: dynamic)
{
  return cpp_expression("#include <bi");
}

func getchar_unlocked(argument_0: dynamic)
{
  return getchar();
}

func putchar_unlocked(c: dynamic)
{
  putchar(c);
}

func mygc(c: dynamic)
{
  return cpp_expression("#include <bits/stdc++.");
}

func mypc(c: dynamic)
{
  return cpp_expression("#include <bits/stdc");
}

func reader(x: dynamic)
{
  var k: dynamic;
  var m = 0;
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

func reader(x: dynamic)
{
  var k: dynamic;
  var m = 0;
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

func reader(c: dynamic)
{
  var i: dynamic;
  var s = 0;
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

func reader(c: dynamic)
{
  var i: dynamic;
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

func reader(x: dynamic, y: dynamic)
{
  reader(x);
  reader(y);
}

func reader(x: dynamic, y: dynamic, z: dynamic)
{
  reader(x);
  reader(y);
  reader(z);
}

func reader(x: dynamic, y: dynamic, z: dynamic, w: dynamic)
{
  reader(x);
  reader(y);
  reader(z);
  reader(w);
}

func writer(x: dynamic, c: dynamic)
{
  var s = 0;
  var m = 0;
  var f = cpp_array(10);
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

func writer(x: dynamic, c: dynamic)
{
  var s = 0;
  var m = 0;
  var f = cpp_array(20);
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

func writer(c: dynamic)
{
  var i: dynamic;
  {
    i = 0;
    while ((c[i] != cpp_char("\u{0}")))
    {
      mypc(c[i]);
      i += 1;
    }
  }
}

func writer(x: dynamic, c: dynamic)
{
  var i: dynamic;
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

func writer(x: dynamic, c: dynamic)
{
  var i: dynamic;
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

func writerLn(x: dynamic)
{
  writer(x, cpp_char("\n"));
}

func writerLn(x: dynamic, y: dynamic)
{
  writer(x, cpp_char(" "));
  writer(y, cpp_char("\n"));
}

func writerLn(x: dynamic, y: dynamic, z: dynamic)
{
  writer(x, cpp_char(" "));
  writer(y, cpp_char(" "));
  writer(z, cpp_char("\n"));
}

func writerArr(x: dynamic, n: dynamic)
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

func writerArr(x: dynamic)
{
  writerArr(x.data(), cpp_cast(x.size()));
}

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
  }
}

func gcd(a: dynamic, b: dynamic)
{
  return if (b) gcd(b, (a % b)) else a;
}

func lcm(a: dynamic, b: dynamic)
{
  return ((a / gcd(a, b)) * b);
}

func mod_pow(a: dynamic, n: dynamic, mod: dynamic)
{
  var ret = 1;
  var p = (a % mod);
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

func extgcd(a: dynamic, b: dynamic, x: dynamic, y: dynamic)
{
  {
    var u = cpp_assign(y, "=", 1);
    var v = cpp_assign(x, "=", 0);
    while (a)
    {
      var q = (b / a);
      swap(cpp_assign(x, "-=", (q * u)), u);
      swap(cpp_assign(y, "-=", (q * v)), v);
      swap(cpp_assign(b, "-=", (q * a)), a);
    }
  }
  return b;
}

func mod_inv(a: dynamic, m: dynamic)
{
  var x: dynamic;
  var y: dynamic;
  extgcd(a, m, x, y);
  return (((m + (x % m))) % m);
}

class UnionFind
{
  var n: dynamic;
  var a: dynamic;
  func UnionFind(n: dynamic)
  {
      this->n = cpp_construct(n);
      this->a = cpp_construct(n, -1);
    }
  func find(x: dynamic)
  {
      return if ((a[x] < 0)) x else (cpp_assign(a[x], "=", find(a[x])));
    }
  func same(x: dynamic, y: dynamic)
  {
      return (find(x) == find(y));
    }
  func same(p: dynamic)
  {
      return same(p.first, p.second);
    }
  func unite(x: dynamic, y: dynamic)
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
  func unite(p: dynamic)
  {
      return unite(p.first, p.second);
    }
  func size()
  {
      return n;
    }
  func size(x: dynamic)
  {
      return (-a[find(x)]);
    }
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  reader(n, m, k);
  FOR(i, n);
  reader(c[i]);
  var vp: dynamic;
  var uf = cpp_construct((n + k));
  var free = 0;
  var ans = 0;
  sort(vp.begin(), vp.end());
  for (var wab in vp)
  {
    if ((uf.size() <= (free + 1)))
    {
      break;
    }
    var w: dynamic;
    var a: dynamic;
    var b: dynamic;
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

func FOR(argument_0: dynamic, argument_1: dynamic)
{
    var a: dynamic;
    var b: dynamic;
    var w: dynamic;
    reader(a, b, w);
    a -= 1;
    b -= 1;
    vp.emplace_back(w, a, b);
  }

func FOR(argument_0: dynamic, argument_1: dynamic)
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
