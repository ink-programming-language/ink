// Translated from solution.cpp.

func rep(i: dynamic, a: dynamic) -> dynamic
{
  cpp_macro("for (int i = 0; i < (int)(a); i++)");
}

func sz(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/");
}

var pcnt: dynamic = cpp_expression("#include <bits/stdc+");

func operator_shift_right(i: dynamic, v: dynamic) -> dynamic
{
  rep(j, sz(v));
  (i >> v[j]);
  return i;
}

func join(v: dynamic) -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  ((rep(i, sz(v)) << cpp_char(" ")) << v[i]);
  return s.str().substr(1);
}

func operator_shift_left(o: dynamic, v: dynamic) -> dynamic
{
  if (sz(v))
  {
    (o << join(v));
  }
  return o;
}

func operator_shift_right(i: dynamic, v: dynamic) -> dynamic
{
  return ((i >> v.first) >> v.second);
}

func operator_shift_left(o: dynamic, v: dynamic) -> dynamic
{
  return (((o << v.first) << ",") << v.second);
}

func mins(x: dynamic, y: dynamic) -> dynamic
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

func maxs(x: dynamic, y: dynamic) -> dynamic
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

func suma(a: dynamic) -> dynamic
{
  var res: dynamic = cpp_construct(0);
  for (var x: dynamic in a)
  {
    res += x;
  }
  return res;
}

func dump() -> dynamic
{
  write("\n");
}

func dump(head: dynamic) -> dynamic
{
  write(head);
  dump();
}

func dump(head: dynamic, tail: dynamic...) -> dynamic
{
  write(head, ", ");
  dump(cpp_expand(forward(tail)));
}

func debug() -> dynamic
{
  cpp_macro("do { cerr << __LINE__ << \":\\t\" << #__VA_ARGS__ << \" = \"; dump(__VA_ARGS__); } while (false)");
}

func dump() -> dynamic
{
  cpp_macro("");
}

func debug() -> dynamic
{
  cpp_macro("");
}

class edge
{
  var src: dynamic = cpp_uninitialized();
  var to: dynamic = cpp_uninitialized();
  var cost: dynamic = cpp_uninitialized();
  func edge(to: dynamic, cost: dynamic) -> dynamic
  {
      self->src = cpp_construct(-1);
      self->to = cpp_construct(to);
      self->cost = cpp_construct(cost);
    }
  func edge(src: dynamic, to: dynamic, cost: dynamic) -> dynamic
  {
      self->src = cpp_construct(src);
      self->to = cpp_construct(to);
      self->cost = cpp_construct(cost);
    }
  func operator_assign(x: dynamic) -> dynamic
  {
      to = x;
      return (*self);
    }
  func cpp_function_1() -> dynamic
  {
      return to;
    }
}

var LINF: dynamic = (1 << 60);

var INF: dynamic = 1001001001;

class ModInt
{
  var x: dynamic = cpp_uninitialized();
  func ModInt(x: dynamic = 0) -> dynamic
  {
      self->x = cpp_construct(((((x % mod) + mod)) % mod));
    }
  func operator_subtract() -> dynamic
  {
      return ModInt((-x));
    }
  func operator_add_assign(a: dynamic) -> dynamic
  {
      if (((cpp_assign(x, "+=", a.x)) >= mod))
      {
        x -= mod;
      }
      return (*self);
    }
  func operator_subtract_assign(a: dynamic) -> dynamic
  {
      if (((cpp_assign(x, "+=", (mod - a.x))) >= mod))
      {
        x -= mod;
      }
      return (*self);
    }
  func operator(a: dynamic) -> dynamic
  {
      (cpp_assign(x, "*=", a.x)) %= mod;
      return (*self);
    }
  func operator_add(a: dynamic) -> dynamic
  {
      return cpp_assign(ModInt((*self)), "+=", a);
    }
  func operator_subtract(a: dynamic) -> dynamic
  {
      return cpp_assign(ModInt((*self)), "-=", a);
    }
  func operator_multiply(a: dynamic) -> dynamic
  {
      return cpp_assign(ModInt((*self)), "*=", a);
    }
  func pow(t: dynamic) -> dynamic
  {
      if ((!t))
      {
        return 1;
      }
      var a: dynamic = (*self);
      var r: dynamic = 1;
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
  func inv() -> dynamic
  {
      var a: dynamic = x;
      var b: dynamic = mod;
      var c: dynamic = 1;
      var d: dynamic = 0;
      while (b)
      {
        var t: dynamic = (a / b);
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
  func operator(a: dynamic) -> dynamic
  {
      return cpp_assign(((*self)), "*=", a.inv());
    }
  func operator_divide(a: dynamic) -> dynamic
  {
      return cpp_assign(ModInt((*self)), "/=", a);
    }
  func operator_equal(a: dynamic) -> dynamic
  {
      return (x == a.x);
    }
  func operator_not_equal(a: dynamic) -> dynamic
  {
      return (x != a.x);
    }
}

var mod: dynamic = 1000000007;

var res: dynamic = cpp_construct(200030);

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  scanf("%d %d", (&n), (&m));
  var ans: dynamic = 0;
  while ((n > 0))
  {
    var t: dynamic = (n % 10);
    ans += res[(m + t)].x;
    n /= 10;
  }
  printf("%lld\n", (ans % mod));
}

func main() -> dynamic
{
  var dp: dynamic = cpp_construct(200030, vector(10));
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
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
