// Translated from solution.cpp.

var mod = 998244353;

var inf = (mod * mod);

var d2 = (((mod + 1)) / 2);

var EPS = 1e-9;

var INF = 1e+10;

var PI = acos(-1.0);

var C_SIZE = 3100000;

var UF_SIZE = 3100000;

var fact = cpp_array(C_SIZE);

var finv = cpp_array(C_SIZE);

var inv = cpp_array(C_SIZE);

func Comb(a: dynamic, b: dynamic)
{
  if (((a < b) || (b < 0)))
  {
    return 0;
  }
  return ((((fact[a] * finv[b]) % mod) * finv[(a - b)]) % mod);
}

func init_C(n: dynamic)
{
  fact[0] = cpp_assign(finv[0], "=", cpp_assign(inv[1], "=", 1));
  {
    var i = 2;
    while ((i < n))
    {
      inv[i] = (((mod - ((((mod / i)) * inv[(mod % i)]) % mod))) % mod);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      fact[i] = ((fact[(i - 1)] * i) % mod);
      finv[i] = ((finv[(i - 1)] * inv[i]) % mod);
      i += 1;
    }
  }
}

func pw(a: dynamic, b: dynamic)
{
  if ((a < 0))
  {
    return 0;
  }
  if ((b < 0))
  {
    return 0;
  }
  var ret = 1;
  while (b)
  {
    if ((b % 2))
    {
      ret = ((ret * a) % mod);
    }
    a = ((a * a) % mod);
    b /= 2;
  }
  return ret;
}

func pw_mod(a: dynamic, b: dynamic, M: dynamic)
{
  if ((a < 0))
  {
    return 0;
  }
  if ((b < 0))
  {
    return 0;
  }
  var ret = 1;
  while (b)
  {
    if ((b % 2))
    {
      ret = ((ret * a) % M);
    }
    a = ((a * a) % M);
    b /= 2;
  }
  return ret;
}

func pw_mod_int(a: dynamic, b: dynamic, M: dynamic)
{
  if ((a < 0))
  {
    return 0;
  }
  if ((b < 0))
  {
    return 0;
  }
  var ret = 1;
  while (b)
  {
    if ((b % 2))
    {
      ret = ((cpp_cast(ret) * a) % M);
    }
    a = ((cpp_cast(a) * a) % M);
    b /= 2;
  }
  return ret;
}

func ABS(a: dynamic)
{
  return max(a, (-a));
}

func ABS(a: dynamic)
{
  return max(a, (-a));
}

func ABS(a: dynamic)
{
  return max(a, (-a));
}

func sig(r: dynamic)
{
  return if (((r < (-EPS)))) -1 else if (((r > (+EPS)))) +1 else 0;
}

var UF = cpp_array(UF_SIZE);

func init_UF(n: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      UF[i] = -1;
      i += 1;
    }
  }
}

func FIND(a: dynamic)
{
  if ((UF[a] < 0))
  {
    return a;
  }
  return cpp_assign(UF[a], "=", FIND(UF[a]));
}

func UNION(a: dynamic, b: dynamic)
{
  a = FIND(a);
  b = FIND(b);
  if ((a == b))
  {
    return;
  }
  UF[a] += UF[b];
  UF[b] = a;
}

var in_cpp = cpp_array(12, 12);

var P = cpp_array(12, 12);

var dp = cpp_array(12, 12, 2);

func f(a: dynamic, b: dynamic)
{
  if (((a + b) == 0))
  {
    return 1;
  }
  {
    var i = 0;
    while ((i < 2))
    {
      {
        var j = 0;
        while ((j < 12))
        {
          {
            var k = 0;
            while ((k < 12))
            {
              dp[i][j][k] = 0;
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  dp[0][0][0] = cpp_assign(dp[1][0][0], "=", 1);
  {
    var i = 0;
    while ((i <= a))
    {
      {
        var j = 0;
        while ((j <= b))
        {
          {
            var k = 0;
            while ((k < 2))
            {
              if ((!dp[k][i][j]))
              {
                k += 1;
                continue;
              }
              if ((k == 0))
              {
                {
                  var l = 1;
                  while (((i + l) <= a))
                  {
                    dp[(!k)][(i + l)][j] = (((dp[(!k)][(i + l)][j] + (dp[k][i][j] * Comb((a - i), l)))) % mod);
                    l += 1;
                  }
                }
              } else
              {
                {
                  var l = 1;
                  while (((j + l) <= b))
                  {
                    dp[(!k)][i][(j + l)] = (((dp[(!k)][i][(j + l)] + (dp[k][i][j] * Comb((b - j), l)))) % mod);
                    l += 1;
                  }
                }
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return (((dp[0][a][b] + dp[1][a][b])) % mod);
}

func main()
{
  var a: dynamic;
  var b: dynamic;
  scanf("%d%d", (&a), (&b));
  {
    var i = 0;
    while ((i < a))
    {
      scanf("%s", in_cpp[i]);
      i += 1;
    }
  }
  init_C(100);
  {
    var i = 0;
    while ((i <= a))
    {
      {
        var j = 0;
        while ((j <= b))
        {
          P[i][j] = f(i, j);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ret = 0;
  {
    var i = 0;
    while ((i < ((1 << a))))
    {
      {
        var j = 0;
        while ((j < ((1 << b))))
        {
          var ok = true;
          var N = a;
          var M = b;
          {
            var k = 0;
            while ((k < a))
            {
              if ((!((i & ((1 << k))))))
              {
                k += 1;
                continue;
              }
              N -= 1;
              var A = 0;
              var B = 0;
              {
                var l = 0;
                while ((l < b))
                {
                  if ((!((j & ((1 << l))))))
                  {
                    l += 1;
                    continue;
                  }
                  if ((in_cpp[k][l] == cpp_char("#")))
                  {
                    A += 1;
                  } else
                  {
                    B += 1;
                  }
                  l += 1;
                }
              }
              if ((((A == 0) || (B == 0))))
              {
                ok = false;
                break;
              }
              k += 1;
            }
          }
          if ((!ok))
          {
            j += 1;
            continue;
          }
          {
            var k = 0;
            while ((k < b))
            {
              if ((!((j & ((1 << k))))))
              {
                k += 1;
                continue;
              }
              M -= 1;
              var A = 0;
              var B = 0;
              {
                var l = 0;
                while ((l < a))
                {
                  if ((!((i & ((1 << l))))))
                  {
                    l += 1;
                    continue;
                  }
                  if ((in_cpp[l][k] == cpp_char("#")))
                  {
                    A += 1;
                  } else
                  {
                    B += 1;
                  }
                  l += 1;
                }
              }
              if ((((A == 0) || (B == 0))))
              {
                ok = false;
                break;
              }
              k += 1;
            }
          }
          if ((!ok))
          {
            j += 1;
            continue;
          }
          if (((N == a) && (M == b)))
          {
            ret = (((ret + P[a][b])) % mod);
          } else
          {
            {
              var k = 0;
              while ((k <= N))
              {
                {
                  var l = 0;
                  while ((l <= M))
                  {
                    var ks = ((Comb(N, k) * Comb(M, l)) % mod);
                    ks = ((ks * P[k][l]) % mod);
                    ks = ((ks * P[(N - k)][(M - l)]) % mod);
                    ret = (((ret + ks)) % mod);
                    l += 1;
                  }
                }
                k += 1;
              }
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%lld\n", ret);
}
