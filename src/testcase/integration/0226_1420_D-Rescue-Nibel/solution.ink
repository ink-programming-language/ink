// Translated from solution.cpp.

var mod = 998244353;

var lmt = (3e5 + 5);

var n: dynamic;

var k: dynamic;

var fac = cpp_array(lmt);

var inv = cpp_array(lmt);

var a = cpp_array(lmt);

var m: dynamic;

var s: dynamic;

func bigmod(n: dynamic, p: dynamic)
{
  if ((p == 0))
  {
    return 1;
  }
  var x = (bigmod(n, (p / 2)) % mod);
  x = (((x * x)) % mod);
  if ((p % 2))
  {
    x = (((x * n)) % mod);
  }
  return x;
}

func ncr(n: dynamic, r: dynamic)
{
  if ((r > n))
  {
    return 0;
  }
  var res = (fac[n] % mod);
  res = (((res * inv[(n - r)])) % mod);
  res = (((res * inv[r])) % mod);
  return res;
}

func main()
{
  ios_base.sync_with_stdio(false);
  fac[0] = cpp_assign(inv[0], "=", 1);
  {
    var i = 1;
    while ((i < lmt))
    {
      fac[i] = (((fac[(i - 1)] * i)) % mod);
      inv[i] = (bigmod(fac[i], (mod - 2)) % mod);
      i += 1;
    }
  }
  read(n, k);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i].first, a[i].second);
      s.insert(a[i].first);
      s.insert(a[i].second);
      i += 1;
    }
  }
  var indx = 0;
  for (var x in s)
  {
    indx += 1;
    m[x] = indx;
  }
  {
    var i = 1;
    while ((i <= n))
    {
      a[i].first = m[a[i].first];
      a[i].second = m[a[i].second];
      i += 1;
    }
  }
  var start = cpp_array((indx + 1));
  var end = cpp_array((indx + 1));
  {
    var i = 1;
    while ((i <= n))
    {
      start[a[i].first].push_back(i);
      end[a[i].second].push_back(i);
      i += 1;
    }
  }
  var s: dynamic;
  s.clear();
  var ans = 0;
  {
    var i = 1;
    while ((i <= indx))
    {
      for (var x in start[i])
      {
        ans = (((ans + ncr(s.size(), (k - 1)))) % mod);
        s.insert(x);
      }
      for (var x in end[i])
      {
        s.erase(x);
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
