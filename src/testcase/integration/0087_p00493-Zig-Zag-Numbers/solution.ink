// Translated from solution.cpp.

var mod: dynamic = 10000;

class Mod
{
  var num: dynamic = cpp_uninitialized();
  func Mod() -> dynamic
  {
      cpp_base_construct(0);
    }
  func Mod(n: dynamic) -> dynamic
  {
      self->num = cpp_construct(((((n % mod) + mod)) % mod));
    }
  func Mod(n: dynamic) -> dynamic
  {
      cpp_base_construct(static_cast(n));
    }
  func cpp_function_1() -> dynamic
  {
      return num;
    }
}

func operator_add(a: dynamic, b: dynamic) -> dynamic
{
  return Mod((((a.num + b.num)) % mod));
}

func operator_add(a: dynamic, b: dynamic) -> dynamic
{
  return (Mod(a) + b);
}

func operator_add(a: dynamic, b: dynamic) -> dynamic
{
  return (b + a);
}

func operator(a: dynamic) -> dynamic
{
  return (a + Mod(1));
}

func operator_subtract(a: dynamic, b: dynamic) -> dynamic
{
  return Mod(((((mod + a.num) - b.num)) % mod));
}

func operator_subtract(a: dynamic, b: dynamic) -> dynamic
{
  return (Mod(a) - b);
}

func operator(a: dynamic) -> dynamic
{
  return (a - Mod(1));
}

func operator_multiply(a: dynamic, b: dynamic) -> dynamic
{
  return Mod((((cpp_cast(a.num) * b.num)) % mod));
}

func operator_multiply(a: dynamic, b: dynamic) -> dynamic
{
  return (Mod(a) * b);
}

func operator_multiply(a: dynamic, b: dynamic) -> dynamic
{
  return (Mod(b) * a);
}

func operator_multiply(a: dynamic, b: dynamic) -> dynamic
{
  return (Mod(b) * a);
}

func operator_add_assign(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a + b));
}

func operator_add_assign(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a + b));
}

func operator_subtract_assign(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a - b));
}

func operator_subtract_assign(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a - b));
}

func operator(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a * b));
}

func operator(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a * b));
}

func operator(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a * b));
}

func operator(a: dynamic, n: dynamic) -> dynamic
{
  if ((n == 0))
  {
    return Mod(1);
  }
  var res: dynamic = (((a * a)) ^ ((n / 2)));
  if ((n % 2))
  {
    res = (res * a);
  }
  return res;
}

func mod_pow(a: dynamic, n: dynamic) -> dynamic
{
  if ((n == 0))
  {
    return Mod(1);
  }
  var res: dynamic = mod_pow(((a * a)), ((n / 2)));
  if ((n % 2))
  {
    res = (res * a);
  }
  return res;
}

func inv(a: dynamic) -> dynamic
{
  return (a ^ ((mod - 2)));
}

func operator_divide(a: dynamic, b: dynamic) -> dynamic
{
  assert((b.num != 0));
  return (a * inv(b));
}

func operator_divide(a: dynamic, b: dynamic) -> dynamic
{
  return (Mod(a) / b);
}

func operator(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", (a / b));
}

var MAX_MOD_N: dynamic = cpp_expression("#includ");

var fact: dynamic = cpp_array(MAX_MOD_N);

var factinv: dynamic = cpp_array(MAX_MOD_N);

func init() -> dynamic
{
  fact[0] = Mod(1);
  factinv[0] = 1;
  {
    var i: dynamic = 0;
    while ((i < (MAX_MOD_N - 1)))
    {
      fact[(i + 1)] = (fact[i] * Mod((i + 1)));
      factinv[(i + 1)] = (factinv[i] / Mod((i + 1)));
      i += 1;
    }
  }
}

func comb(a: dynamic, b: dynamic) -> dynamic
{
  return ((fact[a] * factinv[b]) * factinv[(a - b)]);
}

var dp: dynamic = cpp_array(2, 500, 2, 2, 2, 10, 502);

func powmod(a: dynamic, b: dynamic, mod: dynamic) -> dynamic
{
  assert((b >= 0));
  if ((b == 0))
  {
    return 1;
  }
  if ((b == 1))
  {
    return a;
  }
  var ans: dynamic = 1;
  var aa: dynamic = powmod(a, (b / 2), mod);
  ans *= (aa * aa);
  ans %= mod;
  if ((b % 2))
  {
    ans *= a;
  }
  ans %= mod;
  return ans;
}

func main() -> dynamic
{
  var A: dynamic = cpp_uninitialized();
  var B: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  read(A, B, M);
  var asize: dynamic = A.size();
  var bsize: dynamic = B.size();
  {
    var i: dynamic = asize;
    while ((i <= 501))
    {
      A = (cpp_char("0") + A);
      i += 1;
    }
  }
  {
    var i: dynamic = bsize;
    while ((i <= 501))
    {
      B = (cpp_char("0") + B);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 10))
    {
      var amore: dynamic = (i >= ((A[501] - cpp_char("0"))));
      var bless: dynamic = (i <= ((B[501] - cpp_char("0"))));
      dp[0][i][0][amore][bless][(i % M)][0] += 1;
      dp[0][i][1][amore][bless][(i % M)][0] += 1;
      dp[0][i][0][amore][bless][(i % M)][1] += 1;
      i += 1;
    }
  }
  {
    var keta: dynamic = 1;
    while ((keta <= 501))
    {
      {
        var prenum: dynamic = 0;
        while ((prenum < 10))
        {
          {
            var upping: dynamic = 0;
            while ((upping < 2))
            {
              {
                var amore: dynamic = 0;
                while ((amore < 2))
                {
                  {
                    var bless: dynamic = 0;
                    while ((bless < 2))
                    {
                      {
                        var bai: dynamic = 0;
                        while ((bai < M))
                        {
                          {
                            var death: dynamic = 0;
                            while ((death < 2))
                            {
                              if (dp[(keta - 1)][prenum][upping][amore][bless][bai][death])
                              {
                                {
                                  var nextnum: dynamic = 0;
                                  while ((nextnum < 10))
                                  {
                                    if (death)
                                    {
                                      if (nextnum)
                                      {
                                        nextnum += 1;
                                        continue;
                                      }
                                    } else
                                    {
                                      if (upping)
                                      {
                                        if ((nextnum <= prenum))
                                        {
                                          nextnum += 1;
                                          continue;
                                        }
                                      } else
                                      {
                                        if ((nextnum >= prenum))
                                        {
                                          nextnum += 1;
                                          continue;
                                        }
                                      }
                                    }
                                    var nextamore: dynamic = cpp_uninitialized();
                                    var nextbless: dynamic = cpp_uninitialized();
                                    if ((nextnum > ((A[(501 - keta)] - cpp_char("0")))))
                                    {
                                      nextamore = true;
                                    } else if ((nextnum == ((A[(501 - keta)] - cpp_char("0")))))
                                    {
                                      nextamore = amore;
                                    } else
                                    {
                                      nextamore = false;
                                    }
                                    if ((nextnum > ((B[(501 - keta)] - cpp_char("0")))))
                                    {
                                      nextbless = false;
                                    } else if ((nextnum == ((B[(501 - keta)] - cpp_char("0")))))
                                    {
                                      nextbless = bless;
                                    } else
                                    {
                                      nextbless = true;
                                    }
                                    var nextbai: dynamic = (bai + (powmod(10, keta, M) * nextnum));
                                    nextbai %= M;
                                    assert((nextbai < mod));
                                    if (((((((keta == 1) && (nextnum == 1)) && (((!upping)) == 1)) && (nextamore == 1)) && (nextbless == 1)) && (nextbai == 0)))
                                    {
                                      var c: dynamic = 0;
                                      c += 1;
                                    }
                                    dp[keta][nextnum][(!upping)][nextamore][nextbless][nextbai][death] += dp[(keta - 1)][prenum][upping][amore][bless][bai][death];
                                    if (((!death) && nextnum))
                                    {
                                      dp[keta][nextnum][(!upping)][nextamore][nextbless][nextbai][1] += dp[(keta - 1)][prenum][upping][amore][bless][bai][death];
                                    }
                                    nextnum += 1;
                                  }
                                }
                              }
                              death += 1;
                            }
                          }
                          bai += 1;
                        }
                      }
                      bless += 1;
                    }
                  }
                  amore += 1;
                }
              }
              upping += 1;
            }
          }
          prenum += 1;
        }
      }
      keta += 1;
    }
  }
  {
    var prenum: dynamic = 0;
    while ((prenum < 10))
    {
      {
        var upping: dynamic = 0;
        while ((upping < 2))
        {
          {
            var death: dynamic = 1;
            while ((death < 2))
            {
              if (dp[501][prenum][upping][1][1][0][death])
              {
                ans += dp[501][prenum][upping][1][1][0][death];
              }
              death += 1;
            }
          }
          upping += 1;
        }
      }
      prenum += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
