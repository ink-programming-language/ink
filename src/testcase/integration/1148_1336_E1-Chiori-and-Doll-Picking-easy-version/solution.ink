// Translated from solution.cpp.

func maxtt(t1: dynamic, t2: dynamic)
{
  t1 = max(t1, t2);
}

func mintt(t1: dynamic, t2: dynamic)
{
  t1 = min(t1, t2);
}

var debug = 0;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var direc = "RDLU";

var MOD2 = (cpp_cast(998244353) * cpp_cast(998244353));

var ln: dynamic;

var lk: dynamic;

var lm: dynamic;

func etp(f: dynamic = 0)
{
  puts(if (f) "YES" else "NO");
  exit(0);
}

func addmod(x: dynamic, y: dynamic, mod: dynamic = 998244353)
{
  x += y;
  if ((x >= mod))
  {
    x -= mod;
  }
  if ((x < 0))
  {
    x += mod;
  }
  assert(((x >= 0) && (x < mod)));
}

func et(x: dynamic = -1)
{
  printf("%d\n", x);
  exit(0);
}

func fastPow(x: dynamic, y: dynamic, mod: dynamic = 998244353)
{
  var ans = 1;
  while ((y > 0))
  {
    if ((y & 1))
    {
      ans = (((x * ans)) % mod);
    }
    x = ((x * x) % mod);
    y >>= 1;
  }
  return ans;
}

func gcd1(x: dynamic, y: dynamic)
{
  return if (y) gcd1(y, (x % y)) else x;
}

var a = cpp_array(200135);

class lsp
{
  var a: dynamic = cpp_array(60);
  var maxBit: dynamic;
  func insert(x: dynamic)
  {
      {
        var i = maxBit;
        while ((~i))
        {
          if ((x & ((1 << i))))
          {
            if ((a[i] != 0))
            {
              x ^= a[i];
            } else
            {
              {
                int_cpp(j) = 0;
                while (((j) < cpp_cast((i))))
                {
                  if ((x & ((1 << j))))
                  {
                    x ^= a[j];
                  }
                  (j) += 1;
                }
              }
              {
                var j = (i + 1);
                while ((j <= maxBit))
                {
                  if ((a[j] & ((1 << i))))
                  {
                    a[j] ^= x;
                  }
                  j += 1;
                }
              }
              a[i] = x;
              return 1;
            }
          }
          i -= 1;
        }
      }
      return 0;
    }
  func getOrthogonal(m: dynamic)
  {
      var res: dynamic;
      var vp: dynamic;
      {
        var j = (m - 1);
        while ((j >= 0))
        {
          if ((!a[j]))
          {
            vp.push_back(j);
            res.a[j] |= (1 << j);
          }
          j -= 1;
        }
      }
      {
        var j = (m - 1);
        while ((j >= 0))
        {
          if (a[j])
          {
            var cc = 0;
            {
              var z = (m - 1);
              while ((z >= 0))
              {
                if ((!a[z]))
                {
                  var w = (((a[j] >> z)) & 1);
                  res.a[vp[cc]] |= (w << j);
                  cc += 1;
                }
                z -= 1;
              }
            }
          }
          j -= 1;
        }
      }
      return res;
    }
}

var sp: dynamic;

var p = cpp_array(66);

var q = cpp_array(66);

func ppt()
{
  {
    var i = 0;
    while ((i <= m))
    {
      printf("%lld ", ((cpp_cast(p[i]) * fastPow(2, (n - k))) % 998244353));
      i += 1;
    }
  }
  exit(0);
}

var bs: dynamic;

func dfs(p: dynamic, k: dynamic, i: dynamic, x: dynamic)
{
  if ((i == k))
  {
    p[builtin_popcountll(x)] += 1;
    return;
  }
  dfs(p, k, (i + 1), x);
  dfs(p, k, (i + 1), (x ^ bs[i]));
}

func calsm(sp: dynamic, p: dynamic, k: dynamic)
{
  bs.clear();
  {
    int_cpp(j) = 0;
    while (((j) < cpp_cast((sp.maxBit))))
    {
      if (sp.a[j])
      {
        bs.push_back(sp.a[j]);
      }
      (j) += 1;
    }
  }
  assert((bs.size() == k));
  dfs(p, k, 0, 0);
}

func calbg()
{
  var C = cpp_construct(60, vector(60, 0));
  C[0][0] = 1;
  {
    var i = 1;
    while ((i <= 55))
    {
      C[i][0] = cpp_assign(C[i][i], "=", 1);
      {
        var j = 1;
        while ((j < i))
        {
          C[i][j] = (((C[(i - 1)][(j - 1)] + C[(i - 1)][j])) % 998244353);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var w = cpp_construct((m + 1), vector((m + 1), 0));
  {
    var c = 0;
    while ((c <= m))
    {
      {
        var d = 0;
        while ((d <= m))
        {
          {
            var j = 0;
            while ((j <= c))
            {
              var val = ((cpp_cast(C[d][j]) * C[(m - d)][(c - j)]) % 998244353);
              if (((j % 2) == 0))
              {
                addmod(w[c][d], val);
              } else
              {
                addmod(w[c][d], (998244353 - val));
              }
              j += 1;
            }
          }
          d += 1;
        }
      }
      c += 1;
    }
  }
  var B = sp.getOrthogonal(m);
  calsm(B, q, (m - k));
  {
    int_cpp(c) = 0;
    while (((c) < cpp_cast(((m + 1)))))
    {
      {
        var d = 0;
        while ((d <= m))
        {
          addmod(p[c], ((cpp_cast(q[d]) * w[c][d]) % 998244353));
          d += 1;
        }
      }
      (c) += 1;
    }
  }
  var tmp = fastPow(2, (m - k));
  tmp = fastPow(tmp, (998244353 - 2));
  {
    int_cpp(c) = 0;
    while (((c) < cpp_cast(((m + 1)))))
    {
      p[c] = ((cpp_cast(p[c]) * tmp) % 998244353);
      (c) += 1;
    }
  }
  ppt();
}

func fmain(tid: dynamic)
{
  scanf("%d%d", (&n), (&m));
  {
    int_cpp(i) = 1;
    while (((i) <= cpp_cast((n))))
    {
      scanf("%lld", (a + i));
      (i) += 1;
    }
  }
  {
    int_cpp(i) = 1;
    while (((i) <= cpp_cast((n))))
    {
      k += sp.insert(a[i]);
      (i) += 1;
    }
  }
  if ((k <= (m / 2)))
  {
    calsm(sp, p, k);
    ppt();
  }
  calbg();
}

func main()
{
  var t = 1;
  {
    int_cpp(i) = 1;
    while (((i) <= cpp_cast((t))))
    {
      fmain(i);
      (i) += 1;
    }
  }
  return 0;
}
