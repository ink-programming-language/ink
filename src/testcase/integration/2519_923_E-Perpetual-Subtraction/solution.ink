// Translated from solution.cpp.

var MAXN: dynamic = 262144;

var MOD: dynamic = 998244353;

func ADD(x: dynamic, y: dynamic) -> dynamic
{
  x += y;
  if ((x >= MOD))
  {
    x -= MOD;
  }
}

func DEC(x: dynamic, y: dynamic) -> dynamic
{
  x -= y;
  if ((x < 0))
  {
    x += MOD;
  }
}

func add(x: dynamic, y: dynamic) -> dynamic
{
  return  (((x + y) < MOD)) ? (x + y) : ((x + y) - MOD);
}

func dec(x: dynamic, y: dynamic) -> dynamic
{
  return  ((x < y)) ? ((x - y) + MOD) : (x - y);
}

func q_pow(a: dynamic, b: dynamic, p: dynamic = MOD) -> dynamic
{
  var ret: dynamic = 1;
  {
    while (b)
    {
      if ((b & 1))
      {
        ret = ((ret * a) % p);
      }
      a = ((a * a) % p);
      b >>= 1;
    }
  }
  return ret;
}

func q_inv(x: dynamic, p: dynamic = MOD) -> dynamic
{
  return q_pow(x, (p - 2), p);
}

var LIM: dynamic = cpp_uninitialized();

var L: dynamic = cpp_uninitialized();

var rev: dynamic = cpp_array(MAXN);

func NTT(f: dynamic, op: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < LIM))
    {
      rev[i] = (((rev[(i >> 1)] >> 1)) | ((((i & 1)) << ((L - 1)))));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < LIM))
    {
      if ((i < rev[i]))
      {
        swap(f[i], f[rev[i]]);
      }
      i += 1;
    }
  }
  {
    var l: dynamic = 2;
    while ((l <= LIM))
    {
      var wn: dynamic = q_pow(3, (((MOD - 1)) / l));
      if ((op == -1))
      {
        wn = q_inv(wn);
      }
      {
        var i: dynamic = 0;
        while ((i < LIM))
        {
          {
            var j: dynamic = 0;
            var g: dynamic = 1;
            while ((j < ((l >> 1))))
            {
              var x: dynamic = f[(i | j)];
              var y: dynamic = (((1 * g) * f[((i | j) | ((l >> 1)))]) % MOD);
              f[(i | j)] = add(x, y);
              f[((i | j) | ((l >> 1)))] = dec(x, y);
              j += 1;
              g = (((1 * g) * wn) % MOD);
            }
          }
          i += l;
        }
      }
      l <<= 1;
    }
  }
  if ((op == -1))
  {
    var iv: dynamic = q_inv(LIM);
    {
      var i: dynamic = 0;
      while ((i < LIM))
      {
        f[i] = (((1 * f[i]) * iv) % MOD);
        i += 1;
      }
    }
  }
}

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var fac: dynamic = cpp_array(MAXN);

var ifac: dynamic = cpp_array(MAXN);

var p: dynamic = cpp_array(MAXN);

var tmp: dynamic = cpp_array(MAXN);

func main() -> dynamic
{
  scanf("%d%lld", (&N), (&M));
  LIM = 1;
  L = 0;
  while ((LIM <= (N * 2)))
  {
    LIM <<= 1;
    L += 1;
  }
  {
    var i: dynamic = 0;
    while ((i <= N))
    {
      scanf("%d", (&p[i]));
      i += 1;
    }
  }
  fac[0] = 1;
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      fac[i] = (((1 * fac[(i - 1)]) * i) % MOD);
      i += 1;
    }
  }
  ifac[N] = q_inv(fac[N]);
  {
    var i: dynamic = (N - 1);
    while ((i >= 0))
    {
      ifac[i] = (((1 * ifac[(i + 1)]) * ((i + 1))) % MOD);
      i -= 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= N))
    {
      p[i] = (((1 * p[i]) * fac[i]) % MOD);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < LIM))
    {
      tmp[i] = 0;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= (N - i)))
    {
      swap(p[i], p[(N - i)]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= N))
    {
      tmp[i] = ifac[i];
      i += 1;
    }
  }
  NTT(tmp, 1);
  NTT(p, 1);
  {
    var i: dynamic = 0;
    while ((i < LIM))
    {
      p[i] = (((1 * p[i]) * tmp[i]) % MOD);
      i += 1;
    }
  }
  NTT(p, -1);
  {
    var i: dynamic = 0;
    while ((i <= (N - i)))
    {
      swap(p[i], p[(N - i)]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= N))
    {
      p[i] = (((1 * p[i]) * ifac[i]) % MOD);
      i += 1;
    }
  }
  {
    var i: dynamic = (N + 1);
    while ((i < LIM))
    {
      p[i] = 0;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= N))
    {
      p[i] = (((1 * p[i]) * q_pow(q_inv((i + 1)), M)) % MOD);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= N))
    {
      p[i] = (((1 * p[i]) * fac[i]) % MOD);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < LIM))
    {
      tmp[i] = 0;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= (N - i)))
    {
      swap(p[i], p[(N - i)]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= N))
    {
      tmp[i] =  ((i & 1)) ? (MOD - ifac[i]) : ifac[i];
      i += 1;
    }
  }
  NTT(tmp, 1);
  NTT(p, 1);
  {
    var i: dynamic = 0;
    while ((i < LIM))
    {
      p[i] = (((1 * p[i]) * tmp[i]) % MOD);
      i += 1;
    }
  }
  NTT(p, -1);
  {
    var i: dynamic = 0;
    while ((i <= (N - i)))
    {
      swap(p[i], p[(N - i)]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= N))
    {
      p[i] = (((1 * p[i]) * ifac[i]) % MOD);
      i += 1;
    }
  }
  {
    var i: dynamic = (N + 1);
    while ((i < LIM))
    {
      p[i] = 0;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= N))
    {
      printf("%d ", p[i]);
      i += 1;
    }
  }
  return 0;
}
