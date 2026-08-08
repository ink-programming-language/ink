// Translated from solution.cpp.

func ri()
{
  var x: dynamic;
  scanf("%d", (&x));
  return x;
}

func smax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return true;
  }
  return false;
}

func smin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
    return true;
  }
  return false;
}

func pw(a: dynamic, b: dynamic)
{
  var c = 1;
  while (b)
  {
    if ((b & 1))
    {
      c = ((c * a));
    }
    a = (a * a);
    b >>= 1;
  }
  return c;
}

class mint
{
  var mod: dynamic;
  var x: dynamic;
  func mint()
  {
      this->x = cpp_construct(0);
    }
  func mint(x: dynamic)
  {
      this->x = cpp_construct((((((x % mod)) + mod)) % mod));
    }
  func operator_add_assign(a: dynamic)
  {
      if (((cpp_assign(x, "+=", a.x)) >= mod))
      {
        x -= mod;
      }
      return (*this);
    }
  func operator_subtract_assign(a: dynamic)
  {
      if (((cpp_assign(x, "+=", (mod - a.x))) >= mod))
      {
        x -= mod;
      }
      return (*this);
    }
  func operator(a: dynamic)
  {
      (cpp_assign(x, "*=", a.x)) %= mod;
      return (*this);
    }
  func operator_add(a: dynamic)
  {
      return cpp_assign(mint((*this)), "+=", a);
    }
  func operator_subtract(a: dynamic)
  {
      return cpp_assign(mint((*this)), "-=", a);
    }
  func operator_multiply(a: dynamic)
  {
      return cpp_assign(mint((*this)), "*=", a);
    }
  func operator_equal(a: dynamic)
  {
      return (x == a.x);
    }
}

var mod = (1e9 + 7);

func inv(a: dynamic)
{
  return pw(a, (mod - 2));
}

var maxn = 300010;

var p = cpp_array(maxn);

var cnt = cpp_array(maxn);

var n: dynamic;

var res: dynamic;

var c = cpp_array(maxn);

var s = cpp_array(maxn);

var g = cpp_array(maxn);

func f(l: dynamic, r: dynamic)
{
  if ((((r < l) || (r < 0)) || (l > (n - 2))))
  {
    return 0;
  }
  if ((l < 0))
  {
    l = 0;
  }
  if ((r > (n - 2)))
  {
    r = (n - 2);
  }
  return (s[r] - (if ((l == 0)) 0 else s[(l - 1)]));
}

func calc(v: dynamic)
{
  v[0] = n;
  {
    var i = (1);
    while ((i < (20)))
    {
      v[0] -= v[i];
      i += 1;
    }
  }
  var sum = 0;
  var num = 0;
  {
    var i = 0;
    while ((i < (20)))
    {
      res += (f(num, ((v[i] + num) - 1)) * (((num * i) - sum)));
      res += (f((num - 1), ((v[i] + num) - 2)) * (((num * i) - sum)));
      num += v[i];
      sum += (v[i] * i);
      i += 1;
    }
  }
  sum = 0;
  num = 0;
  {
    var i = 19;
    while ((i >= 0))
    {
      res += (f(num, ((v[i] + num) - 1)) * ((sum - (num * i))));
      res += (f((num + 1), (v[i] + num)) * ((sum - (num * i))));
      num += v[i];
      sum += (v[i] * i);
      i -= 1;
    }
  }
}

func main()
{
  n = ri();
  {
    var i = 0;
    while ((i < (n)))
    {
      cnt[ri()] += 1;
      i += 1;
    }
  }
  c[0] = cpp_assign(s[0], "=", 1);
  {
    var i = 0;
    while ((i < ((n - 2))))
    {
      c[(i + 1)] = ((c[i] * inv(mint((i + 1)))) * (((n - 2) - i)));
      s[(i + 1)] = (c[(i + 1)] + s[i]);
      i += 1;
    }
  }
  {
    var i = (2);
    while ((i < (maxn)))
    {
      if (p[i])
      {
        i += 1;
        continue;
      }
      {
        var j = i;
        while ((j < maxn))
        {
          if ((!p[j]))
          {
            p[j] = i;
          }
          j += i;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (maxn)))
    {
      if (cnt[i])
      {
        var x = i;
        while ((x > 1))
        {
          var t = p[x];
          var cur = 0;
          while (((x % t) == 0))
          {
            x /= t;
            cur += 1;
          }
          if (g[t].empty())
          {
            g[t].resize(20);
          }
          g[t][cur] += cnt[i];
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (maxn)))
    {
      if ((!g[i].empty()))
      {
        calc(g[i]);
      }
      i += 1;
    }
  }
  write(res.x, "\n");
  return 0;
}
