// Translated from solution.cpp.

var PII = cpp_expression("#include <cstdlib> #include <io");

var PTT = cpp_expression("#include <cstdlib> #inclu");

func min(x: dynamic, y: dynamic)
{
  return if ((x < y)) x else y;
}

func max(x: dynamic, y: dynamic)
{
  return if ((x > y)) x else y;
}

func __cpp_top_level_1()
{
}

var INF = 2000000005;

var mod = 998244353;

var MAXN = 180;

class vector
{
  var e: dynamic = cpp_array(MAXN);
  func vector()
  {
      memset(e, 0, cpp_sizeof((e)));
    }
  func operator_multiply(a: dynamic)
  {
      var t = (*this);
      {
        var i = 0;
        while ((i < MAXN))
        {
          t.e[i] = ((t.e[i] * a) % mod);
          i += 1;
        }
      }
      return t;
    }
  func operator_add(t: dynamic)
  {
      {
        var i = 0;
        while ((i < MAXN))
        {
          t.e[i] = (((t.e[i] + e[i])) % mod);
          i += 1;
        }
      }
      return t;
    }
}

class matrix
{
  var c: dynamic = cpp_array(MAXN);
  func operator_multiply(v: dynamic)
  {
      var t: dynamic;
      {
        var i = 0;
        while ((i < MAXN))
        {
          t = (t + (c[i] * v.e[i]));
          i += 1;
        }
      }
      return t;
    }
  func operator_multiply(m: dynamic)
  {
      var t: dynamic;
      {
        var i = 0;
        while ((i < MAXN))
        {
          {
            var j = 0;
            while ((j < MAXN))
            {
              t.c[j].e[i] = 0;
              {
                var k = 0;
                while ((k < MAXN))
                {
                  t.c[j].e[i] += ((c[k].e[i] * m.c[j].e[k]) % mod);
                  k += 1;
                }
              }
              t.c[j].e[i] %= mod;
              j += 1;
            }
          }
          i += 1;
        }
      }
      return t;
    }
  func get(i: dynamic, j: dynamic)
  {
      return c[j].e[i];
    }
  func print(n: dynamic)
  {
      {
        var i = 0;
        while ((i < n))
        {
          {
            var j = 0;
            while ((j < n))
            {
              printf("%lld%c", c[j].e[i], if ((j == (n - 1))) cpp_char("\n") else cpp_char(" "));
              j += 1;
            }
          }
          i += 1;
        }
      }
    }
  func ID()
  {
      var I: dynamic;
      {
        var i = 0;
        while ((i < MAXN))
        {
          I.get(i, i) = 1;
          i += 1;
        }
      }
      return I;
    }
}

func qpow(A: dynamic, x: dynamic)
{
  if ((!x))
  {
    A = matrix.ID();
    return;
  }
  if ((x & 1))
  {
    var t = A;
    qpow(A, (x / 2));
    A = ((A * A) * t);
  } else
  {
    qpow(A, (x / 2));
    A = (A * A);
  }
}

func streql(a: dynamic, b: dynamic, len: dynamic)
{
  {
    var i = 0;
    while ((i < len))
    {
      if ((a[i] != b[i]))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

var A: dynamic;

var N: dynamic;

var M: dynamic;

var len = cpp_array(10);

var str = cpp_array(10, 10);

var id = cpp_array(10, 10, 10);

func id(n: dynamic, i: dynamic, j: dynamic)
{
  return id[n][i][j];
  return (((n * 25) + (i * 5)) + j);
}

func init()
{
  var tt = 0;
  {
    var i = 0;
    while ((i < 8))
    {
      {
        var j = 0;
        while ((j < 5))
        {
          {
            var k = 0;
            while ((k <= j))
            {
              id[i][j][k] = cpp_update(tt, "++");
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  scanf("%d %d", (&N), (&M));
  {
    var i = 0;
    while ((i < N))
    {
      scanf("%s", str[i]);
      len[i] = strlen(str[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      {
        var j = 0;
        while ((j < len[i]))
        {
          {
            var k = 0;
            while ((k <= j))
            {
              if ((j == 0))
              {
                {
                  var s = 0;
                  while ((s < N))
                  {
                    {
                      var t = 0;
                      while ((t < N))
                      {
                        if (((len[t] <= len[s]) && streql(str[s], str[t], len[t])))
                        {
                          A.get(id(s, (len[s] - 1), (len[t] - 1)), id(i, j, k)) += (1 + ((len[t] != len[s])));
                        }
                        t += 1;
                      }
                    }
                    s += 1;
                  }
                }
                k += 1;
                continue;
              }
              if ((k == 0))
              {
                {
                  var s = 0;
                  while ((s < N))
                  {
                    if (((len[s] <= j) && streql(((str[i] + len[i]) - j), str[s], len[s])))
                    {
                      A.get(id(i, (j - 1), (len[s] - 1)), id(i, j, k)) += 1;
                    } else if (((len[s] > j) && streql(((str[i] + len[i]) - j), str[s], j)))
                    {
                      A.get(id(s, (len[s] - 1), (j - 1)), id(i, j, k)) += 1;
                    }
                    s += 1;
                  }
                }
                k += 1;
                continue;
              }
              A.get(id(i, (j - 1), (k - 1)), id(i, j, k)) += 1;
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func solve()
{
  var v: dynamic;
  v.e[id(0, 0, 0)] = 1;
  var m = A;
  qpow(m, M);
  v = (m * v);
  var ans = 0;
  {
    var i = 0;
    while ((i < N))
    {
      ans = (((ans + v.e[id(i, 0, 0)])) % mod);
      i += 1;
    }
  }
  printf("%lld\n", ans);
}

func main()
{
  init();
  solve();
  return 0;
}
