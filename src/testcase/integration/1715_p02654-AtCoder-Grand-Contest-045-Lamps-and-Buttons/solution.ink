// Translated from solution.cpp.

var int_cpp = dynamic;

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

var pb = cpp_expression("#include<");

var eb = cpp_expression("#include<bit");

func all(v: dynamic)
{
  return cpp_expression("#include<bits/stdc++.");
}

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

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

func operator_shift_left(ost: dynamic, p: dynamic)
{
  (((((ost << "{") << p.first) << ",") << p.second) << "}");
  return ost;
}

func operator_shift_left(ost: dynamic, v: dynamic)
{
  (ost << "{");
  {
    var i = 0;
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

func topbit(x: dynamic)
{
  return if (x) (63 - builtin_clzll(x)) else -1;
}

func popcount(x: dynamic)
{
  return builtin_popcountll(x);
}

func parity(x: dynamic)
{
  return builtin_parity(x);
}

class ModInt
{
  var a: dynamic;
  func s(vv: dynamic)
  {
      a = if ((vv < mod)) vv else (vv - mod);
      return (*this);
    }
  func ModInt(x: dynamic = 0)
  {
      s(((x % mod) + mod));
    }
  func operator_add_assign(x: dynamic)
  {
      return s((a + x.a));
    }
  func operator_subtract_assign(x: dynamic)
  {
      return s(((a + mod) - x.a));
    }
  func operator(x: dynamic)
  {
      a = ((uint64_t(a) * x.a) % mod);
      return (*this);
    }
  func operator(x: dynamic)
  {
      (*this) *= x.inv();
      return (*this);
    }
  func operator_add(x: dynamic)
  {
      return cpp_assign(ModInt((*this)), "+=", x);
    }
  func operator_subtract(x: dynamic)
  {
      return cpp_assign(ModInt((*this)), "-=", x);
    }
  func operator_multiply(x: dynamic)
  {
      return cpp_assign(ModInt((*this)), "*=", x);
    }
  func operator_divide(x: dynamic)
  {
      return cpp_assign(ModInt((*this)), "/=", x);
    }
  func operator_equal(x: dynamic)
  {
      return (a == x.a);
    }
  func operator_not_equal(x: dynamic)
  {
      return (a != x.a);
    }
  func operator_less(x: dynamic)
  {
      return (a < x.a);
    }
  func operator_subtract()
  {
      return (ModInt() - (*this));
    }
  func pow(n: dynamic)
  {
      var res = cpp_construct(1);
      var x = cpp_construct((*this));
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
  func inv()
  {
      return pow((mod - 2));
    }
}

func operator_shift_right(in_cpp: dynamic, a: dynamic)
{
  return ((in_cpp >> a.a));
}

func operator_shift_left(out: dynamic, a: dynamic)
{
  return ((out << a.a));
}

class ModIntTable
{
  var N: dynamic;
  var facts: dynamic;
  var finvs: dynamic;
  var invs: dynamic;
  func ModIntTable()
  {
      this->N = cpp_construct((1 << lg));
      this->facts = cpp_construct(N);
      this->finvs = cpp_construct(N);
      this->invs = cpp_construct(N);
      var mod = (Mint(-1).a + 1);
      invs[1] = 1;
      {
        var i = 2;
        while ((i < N))
        {
          invs[i] = (invs[(mod % i)] * ((mod - (mod / i))));
          i += 1;
        }
      }
      facts[0] = 1;
      finvs[0] = 1;
      {
        var i = 1;
        while ((i < N))
        {
          facts[i] = (facts[(i - 1)] * i);
          finvs[i] = (finvs[(i - 1)] * invs[i]);
          i += 1;
        }
      }
    }
  func fact(n: dynamic)
  {
      return facts[n];
    }
  func finv(n: dynamic)
  {
      return finvs[n];
    }
  func inv(n: dynamic)
  {
      return invs[n];
    }
  func binom(n: dynamic, k: dynamic)
  {
      if ((((n < 0) || (k < 0)) || (k > n)))
      {
        return 0;
      }
      return ((facts[n] * finvs[k]) * finvs[(n - k)]);
    }
  func perm(n: dynamic, k: dynamic)
  {
      if ((((n < 0) || (k < 0)) || (k > n)))
      {
        return 0;
      }
      return (facts[n] * finvs[(n - k)]);
    }
  func catalan(n: dynamic)
  {
      return ((facts[(2 * n)] * finvs[(n + 1)]) * finvs[n]);
    }
}

var mt: dynamic;

func main()
{
  var N: dynamic;
  var A: dynamic;
  read(N, A);
  var ans = 0;
  {
    var i = 1;
    while ((i <= A))
    {
      var num = 0;
      var r = max(0, ((A - i) - 1));
      {
        var j = 0;
        while ((j <= i))
        {
          var tmp = ((mt.fact((i - j)) * mt.perm((((i - j) + ((N - A))) - 1), (N - A))) * mt.binom(i, j));
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
