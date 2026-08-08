// Translated from solution.cpp.

class modint
{
  var x: dynamic;
  func modint()
  {
      this->x = cpp_construct(0);
    }
  func modint(arg: dynamic)
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
  func operator_add_assign(other: dynamic)
  {
      x += other.x;
      if ((x >= m))
      {
        x -= m;
      }
      return (*this);
    }
  func operator(other: dynamic)
  {
      x = ((((x * 1) * other.x)) % m);
      return (*this);
    }
  func operator_subtract_assign(other: dynamic)
  {
      x += (m - other.x);
      if ((x >= m))
      {
        x -= m;
      }
      return (*this);
    }
  func operator_add(other: dynamic)
  {
      var tmp = (*this);
      tmp += other;
      return tmp;
    }
  func operator_subtract(other: dynamic)
  {
      var tmp = (*this);
      tmp -= other;
      return tmp;
    }
  func operator_multiply(other: dynamic)
  {
      var tmp = (*this);
      tmp *= other;
      return tmp;
    }
  func cpp_function_1()
  {
      return x;
    }
  func operator()
  {
      x += 1;
      if ((x == m))
      {
        x = 0;
      }
      return (*this);
    }
  func operator()
  {
      if ((x == 0))
      {
        x = (m - 1);
      } else
      {
        x -= 1;
      }
      return (*this);
    }
  func operator(argument_0: dynamic)
  {
      var tmp = (*this);
      (*this) += 1;
      return tmp;
    }
  func operator(argument_0: dynamic)
  {
      var tmp = (*this);
      (*this) -= 1;
      return tmp;
    }
  func operator_equal(other: dynamic)
  {
      return (x == other.x);
    }
  func operator_not_equal(other: dynamic)
  {
      return (x != other.x);
    }
  func operator(arg: dynamic)
  {
      if ((arg == 0))
      {
        return 1;
      }
      if ((arg == 1))
      {
        return x;
      }
      var t = ((*this) ^ ((arg >> 1)));
      t *= t;
      if ((arg & 1))
      {
        t *= (*this);
      }
      return t;
    }
  func operator(arg: dynamic)
  {
      return cpp_assign((*this), "=", ((*this) ^ arg));
    }
  func inv()
  {
      return ((*this) ^ ((m - 2)));
    }
}

var MOD = 998244353;

var n: dynamic;

var m: dynamic;

var w = cpp_array(55);

var t = cpp_array(55);

var w0: dynamic;

var w1: dynamic;

var dp = cpp_array(55, 55, 55);

var inverz = cpp_array(10000);

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  cerr.tie(null);
  {
    var i = 1;
    while ((i < 10000))
    {
      inverz[i] = modint(i).inv();
      i += 1;
    }
  }
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      read(t[i]);
      i += 1;
    }
  }
  {
    var i = 0;
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
    var i = 0;
    while ((i < n))
    {
      memset(dp, 0, cpp_sizeof((dp)));
      dp[0][0][0] = 1;
      var x = w[i];
      var w0 = w0;
      var w1 = w1;
      if ((t[i] == 0))
      {
        w0 -= x;
      } else
      {
        w1 -= x;
      }
      var dir = if ((t[i] == 0)) -1 else 1;
      {
        var p = 0;
        while ((p < m))
        {
          {
            var q = 0;
            while (((p + q) < m))
            {
              {
                var r = 0;
                while ((((p + q) + r) < m))
                {
                  var ukupno = ((((x + (p * dir))) + ((w0 - q))) + ((w1 + r)));
                  if ((((w0 - q) < 0) || ((x + (p * dir)) < 0)))
                  {
                    r += 1;
                    continue;
                  }
                  var ukinv = (inverz[ukupno] * dp[p][q][r]);
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
      var sol: dynamic;
      {
        var p = 0;
        while ((p <= m))
        {
          {
            var q = 0;
            while (((p + q) <= m))
            {
              var r = ((m - p) - q);
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
