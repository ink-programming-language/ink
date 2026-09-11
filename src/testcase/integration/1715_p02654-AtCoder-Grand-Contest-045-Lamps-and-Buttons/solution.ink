// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

var pb: dynamic = cpp_expression("#include<");

var eb: dynamic = cpp_expression("#include<bit");

func all(v: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

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

func operator_shift_left(ost: dynamic, p: dynamic) -> dynamic
{
  (((((ost << "{") << p.first) << ",") << p.second) << "}");
  return ost;
}

func operator_shift_left(ost: dynamic, v: dynamic) -> dynamic
{
  (ost << "{");
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      if (i)
      {
        (ost << ",");
      }
      (ost << v[i]);
      i += 1;
    }
  }
  (ost << "}");
  return ost;
}

func topbit(x: dynamic) -> dynamic
{
  return  (x) ? (63 - builtin_clzll(x)) : -1;
}

func popcount(x: dynamic) -> dynamic
{
  return builtin_popcountll(x);
}

func parity(x: dynamic) -> dynamic
{
  return builtin_parity(x);
}

class ModInt
{
  var a: dynamic = cpp_uninitialized();
  func s(vv: dynamic) -> dynamic
  {
      a =  ((vv < mod)) ? vv : (vv - mod);
      return (*self);
    }
  func ModInt(x: dynamic = 0) -> dynamic
  {
      s(((x % mod) + mod));
    }
  func operator_add_assign(x: dynamic) -> dynamic
  {
      return s((a + x.a));
    }
  func operator_subtract_assign(x: dynamic) -> dynamic
  {
      return s(((a + mod) - x.a));
    }
  func operator(x: dynamic) -> dynamic
  {
      a = ((uint64_t(a) * x.a) % mod);
      return (*self);
    }
  func operator(x: dynamic) -> dynamic
  {
      (*self) *= x.inv();
      return (*self);
    }
  func operator_add(x: dynamic) -> dynamic
  {
      return cpp_assign(ModInt((*self)), "+=", x);
    }
  func operator_subtract(x: dynamic) -> dynamic
  {
      return cpp_assign(ModInt((*self)), "-=", x);
    }
  func operator_multiply(x: dynamic) -> dynamic
  {
      return cpp_assign(ModInt((*self)), "*=", x);
    }
  func operator_divide(x: dynamic) -> dynamic
  {
      return cpp_assign(ModInt((*self)), "/=", x);
    }
  func operator_equal(x: dynamic) -> dynamic
  {
      return (a == x.a);
    }
  func operator_not_equal(x: dynamic) -> dynamic
  {
      return (a != x.a);
    }
  func operator_less(x: dynamic) -> dynamic
  {
      return (a < x.a);
    }
  func operator_subtract() -> dynamic
  {
      return (ModInt() - (*self));
    }
  func pow(n: dynamic) -> dynamic
  {
      var res: dynamic = cpp_construct(1);
      var x: dynamic = cpp_construct((*self));
      while (n)
      {
        if ((n & 1))
        {
          res *= x;
        }
        x *= x;
        n >>= 1;
      }
      return res;
    }
  func inv() -> dynamic
  {
      return pow((mod - 2));
    }
}

func operator_shift_right(in_cpp: dynamic, a: dynamic) -> dynamic
{
  return ((in_cpp >> a.a));
}

func operator_shift_left(out: dynamic, a: dynamic) -> dynamic
{
  return ((out << a.a));
}

class ModIntTable
{
  var N: dynamic = cpp_uninitialized();
  var facts: dynamic = cpp_uninitialized();
  var finvs: dynamic = cpp_uninitialized();
  var invs: dynamic = cpp_uninitialized();
  func ModIntTable() -> dynamic
  {
      self->N = cpp_construct((1 << lg));
      self->facts = cpp_construct(N);
      self->finvs = cpp_construct(N);
      self->invs = cpp_construct(N);
      var mod: dynamic = (Mint(-1).a + 1);
      invs[1] = 1;
      {
        var i: dynamic = 2;
        while ((i < N))
        {
          invs[i] = (invs[(mod % i)] * ((mod - (mod / i))));
          i += 1;
        }
      }
      facts[0] = 1;
      finvs[0] = 1;
      {
        var i: dynamic = 1;
        while ((i < N))
        {
          facts[i] = (facts[(i - 1)] * i);
          finvs[i] = (finvs[(i - 1)] * invs[i]);
          i += 1;
        }
      }
    }
  func fact(n: dynamic) -> dynamic
  {
      return facts[n];
    }
  func finv(n: dynamic) -> dynamic
  {
      return finvs[n];
    }
  func inv(n: dynamic) -> dynamic
  {
      return invs[n];
    }
  func binom(n: dynamic, k: dynamic) -> dynamic
  {
      if ((((n < 0) || (k < 0)) || (k > n)))
      {
        return 0;
      }
      return ((facts[n] * finvs[k]) * finvs[(n - k)]);
    }
  func perm(n: dynamic, k: dynamic) -> dynamic
  {
      if ((((n < 0) || (k < 0)) || (k > n)))
      {
        return 0;
      }
      return (facts[n] * finvs[(n - k)]);
    }
  func catalan(n: dynamic) -> dynamic
  {
      return ((facts[(2 * n)] * finvs[(n + 1)]) * finvs[n]);
    }
}

var mt: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var A: dynamic = cpp_uninitialized();
  read(N, A);
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= A))
    {
      var num: dynamic = 0;
      var r: dynamic = max(0, ((A - i) - 1));
      {
        var j: dynamic = 0;
        while ((j <= i))
        {
          var tmp: dynamic = ((mt.fact((i - j)) * mt.perm((((i - j) + ((N - A))) - 1), (N - A))) * mt.binom(i, j));
          tmp *= mt.perm(((((i - j) + N) - A) + r), r);
          if ((j & 1))
          {
            num -= tmp;
          } else
          {
            num += tmp;
          }
          j += 1;
        }
      }
      ans += num;
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
