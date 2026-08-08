// Translated from solution.cpp.

func rep(i: dynamic, a: dynamic)
{
  cpp_macro("for (int i = 0; i < (int)(a); i++)");
}

func sz(x: dynamic)
{
  return cpp_expression("#include <bits/");
}

var pcnt = cpp_expression("#include <bits/stdc+");

func operator_shift_right(i: dynamic, v: dynamic)
{
  rep(j, sz(v));
  (i >> v[j]);
  return i;
}

func join(v: dynamic)
{
  var s: dynamic;
  ((rep(i, sz(v)) << cpp_char(" ")) << v[i]);
  return s.str().substr(1);
}

func operator_shift_left(o: dynamic, v: dynamic)
{
  if (sz(v))
  {
    (o << join(v));
  }
  return o;
}

func operator_shift_right(i: dynamic, v: dynamic)
{
  return ((i >> v.first) >> v.second);
}

func operator_shift_left(o: dynamic, v: dynamic)
{
  return (((o << v.first) << ",") << v.second);
}

func mins(x: dynamic, y: dynamic)
{
  if ((x > y))
  {
    x = y;
    return true;
  } else
  {
    return false;
  }
}

func maxs(x: dynamic, y: dynamic)
{
  if ((x < y))
  {
    x = y;
    return true;
  } else
  {
    return false;
  }
}

func suma(a: dynamic)
{
  var res = cpp_construct(0);
  for (var x in a)
  {
    res += x;
  }
  return res;
}

func dump()
{
  write("\n");
}

func dump(head: dynamic)
{
  write(head);
  dump();
}

func dump(head: dynamic, tail: dynamic...)
{
  write(head, ", ");
  dump(cpp_expand(forward(tail)));
}

func debug()
{
  cpp_macro("do { cerr << __LINE__ << \":\\t\" << #__VA_ARGS__ << \" = \"; dump(__VA_ARGS__); } while (false)");
}

func dump()
{
  cpp_macro("");
}

func debug()
{
  cpp_macro("");
}

class edge
{
  var src: dynamic;
  var to: dynamic;
  var cost: dynamic;
  func edge(to: dynamic, cost: dynamic)
  {
      this->src = cpp_construct(-1);
      this->to = cpp_construct(to);
      this->cost = cpp_construct(cost);
    }
  func edge(src: dynamic, to: dynamic, cost: dynamic)
  {
      this->src = cpp_construct(src);
      this->to = cpp_construct(to);
      this->cost = cpp_construct(cost);
    }
  func operator_assign(x: dynamic)
  {
      to = x;
      return (*this);
    }
  func cpp_function_1()
  {
      return to;
    }
}

var LINF = (1 << 60);

var INF = 1001001001;

class ModInt
{
  var x: dynamic;
  func ModInt(x: dynamic = 0)
  {
      this->x = cpp_construct(((((x % mod) + mod)) % mod));
    }
  func operator_subtract()
  {
      return ModInt((-x));
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
      return cpp_assign(ModInt((*this)), "+=", a);
    }
  func operator_subtract(a: dynamic)
  {
      return cpp_assign(ModInt((*this)), "-=", a);
    }
  func operator_multiply(a: dynamic)
  {
      return cpp_assign(ModInt((*this)), "*=", a);
    }
  func pow(t: dynamic)
  {
      if ((!t))
      {
        return 1;
      }
      var a = (*this);
      var r = 1;
      while (t)
      {
        if ((t & 1))
        {
          r *= a;
        }
        a *= a;
        t >>= 1;
      }
      return r;
    }
  func inv()
  {
      var a = x;
      var b = mod;
      var c = 1;
      var d = 0;
      while (b)
      {
        var t = (a / b);
        a -= (t * b);
        swap(a, b);
        c -= (t * d);
        swap(c, d);
      }
      c %= mod;
      if ((c < 0))
      {
        c += mod;
      }
      return c;
    }
  func operator(a: dynamic)
  {
      return cpp_assign(((*this)), "*=", a.inv());
    }
  func operator_divide(a: dynamic)
  {
      return cpp_assign(ModInt((*this)), "/=", a);
    }
  func operator_equal(a: dynamic)
  {
      return (x == a.x);
    }
  func operator_not_equal(a: dynamic)
  {
      return (x != a.x);
    }
}

var mod = 1000000007;

var res = cpp_construct(200030);

func solve()
{
  var n: dynamic;
  var m: dynamic;
  scanf("%d %d", (&n), (&m));
  var ans = 0;
  while ((n > 0))
  {
    var t = (n % 10);
    ans += res[(m + t)].x;
    n /= 10;
  }
  printf("%lld\n", (ans % mod));
}

func main()
{
  var dp = cpp_construct(200030, vector(10));
  dp[0][0] = 1;
  rep(j, 200011);
  {
    rep(k, 9)[(j + 1)][(k + 1)] += dp[j][k];
    dp[(j + 1)][0] += dp[j][9];
    dp[(j + 1)][1] += dp[j][9];
    rep(k, 10);
    {
      res[(j + 1)] += dp[(j + 1)][k];
    }
  }
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
