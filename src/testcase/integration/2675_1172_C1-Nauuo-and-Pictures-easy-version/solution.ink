// Translated from solution.cpp.

class modint
{
  var x: dynamic = cpp_uninitialized();
  func modint() -> dynamic
  {
      self->x = cpp_construct(0);
    }
  func modint(arg: dynamic) -> dynamic
  {
      arg %= m;
      if ((arg < 0))
      {
        x = (arg + m);
      } else
      {
        x = arg;
      }
    }
  func operator_add_assign(other: dynamic) -> dynamic
  {
      x += other.x;
      if ((x >= m))
      {
        x -= m;
      }
      return (*self);
    }
  func operator(other: dynamic) -> dynamic
  {
      x = ((((x * 1) * other.x)) % m);
      return (*self);
    }
  func operator_subtract_assign(other: dynamic) -> dynamic
  {
      x += (m - other.x);
      if ((x >= m))
      {
        x -= m;
      }
      return (*self);
    }
  func operator_add(other: dynamic) -> dynamic
  {
      var tmp: dynamic = (*self);
      tmp += other;
      return tmp;
    }
  func operator_subtract(other: dynamic) -> dynamic
  {
      var tmp: dynamic = (*self);
      tmp -= other;
      return tmp;
    }
  func operator_multiply(other: dynamic) -> dynamic
  {
      var tmp: dynamic = (*self);
      tmp *= other;
      return tmp;
    }
  func cpp_function_1() -> dynamic
  {
      return x;
    }
  func operator() -> dynamic
  {
      x += 1;
      if ((x == m))
      {
        x = 0;
      }
      return (*self);
    }
  func operator() -> dynamic
  {
      if ((x == 0))
      {
        x = (m - 1);
      } else
      {
        x -= 1;
      }
      return (*self);
    }
  func operator(argument_0: dynamic) -> dynamic
  {
      var tmp: dynamic = (*self);
      (*self) += 1;
      return tmp;
    }
  func operator(argument_0: dynamic) -> dynamic
  {
      var tmp: dynamic = (*self);
      (*self) -= 1;
      return tmp;
    }
  func operator_equal(other: dynamic) -> dynamic
  {
      return (x == other.x);
    }
  func operator_not_equal(other: dynamic) -> dynamic
  {
      return (x != other.x);
    }
  func operator(arg: dynamic) -> dynamic
  {
      if ((arg == 0))
      {
        return 1;
      }
      if ((arg == 1))
      {
        return x;
      }
      var t: dynamic = ((*self) ^ ((arg >> 1)));
      t *= t;
      if ((arg & 1))
      {
        t *= (*self);
      }
      return t;
    }
  func operator(arg: dynamic) -> dynamic
  {
      return cpp_assign((*self), "=", ((*self) ^ arg));
    }
  func inv() -> dynamic
  {
      return ((*self) ^ ((m - 2)));
    }
}

var MOD: dynamic = 998244353;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var w: dynamic = cpp_array(55);

var t: dynamic = cpp_array(55);

var w0: dynamic = cpp_uninitialized();

var w1: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(55, 55, 55);

var inverz: dynamic = cpp_array(10000);

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  cerr.tie(null);
  {
    var i: dynamic = 1;
    while ((i < 10000))
    {
      inverz[i] = modint(i).inv();
      i += 1;
    }
  }
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(t[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(w[i]);
      if ((t[i] == 0))
      {
        w0 += w[i];
      } else
      {
        w1 += w[i];
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      memset(dp, 0, cpp_sizeof((dp)));
      dp[0][0][0] = 1;
      var x: dynamic = w[i];
      var w0: dynamic = w0;
      var w1: dynamic = w1;
      if ((t[i] == 0))
      {
        w0 -= x;
      } else
      {
        w1 -= x;
      }
      var dir: dynamic =  ((t[i] == 0)) ? -1 : 1;
      {
        var p: dynamic = 0;
        while ((p < m))
        {
          {
            var q: dynamic = 0;
            while (((p + q) < m))
            {
              {
                var r: dynamic = 0;
                while ((((p + q) + r) < m))
                {
                  var ukupno: dynamic = ((((x + (p * dir))) + ((w0 - q))) + ((w1 + r)));
                  if ((((w0 - q) < 0) || ((x + (p * dir)) < 0)))
                  {
                    r += 1;
                    continue;
                  }
                  var ukinv: dynamic = (inverz[ukupno] * dp[p][q][r]);
                  dp[(p + 1)][q][r] += (ukinv * modint((x + (p * dir))));
                  dp[p][(q + 1)][r] += (ukinv * modint((w0 - q)));
                  dp[p][q][(r + 1)] += (ukinv * modint((w1 + r)));
                  r += 1;
                }
              }
              q += 1;
            }
          }
          p += 1;
        }
      }
      var sol: dynamic = cpp_uninitialized();
      {
        var p: dynamic = 0;
        while ((p <= m))
        {
          {
            var q: dynamic = 0;
            while (((p + q) <= m))
            {
              var r: dynamic = ((m - p) - q);
              sol += (dp[p][q][r] * ((x + (p * dir))));
              q += 1;
            }
          }
          p += 1;
        }
      }
      write(cpp_cast(sol), cpp_char("\n"));
      i += 1;
    }
  }
}
