// Translated from solution.cpp.

var ll = dynamic;

var maxn = cpp_expression("#inclu");

var mod = cpp_expression("#incl");

var db = cpp_expression("#inclu");

var vi = cpp_expression("#include <b");

var pb = cpp_expression("#include");

var mp = cpp_expression("#include");

var pi = cpp_expression("#include <bits");

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

func ksm(a: dynamic, b: dynamic)
{
  if ((!b))
  {
    return 1;
  }
  var ns = ksm(a, (b >> 1));
  ns = ((ns * ns) % mod);
  if ((b & 1))
  {
    ns = ((ns * a) % mod);
  }
  return ns;
}

func add(a: dynamic, b: dynamic)
{
  a += b;
  return if (((a < mod))) a else ((a - mod));
}

func sub(a: dynamic, b: dynamic)
{
  a -= b;
  return if (((a < 0))) (a + mod) else a;
}

func mul(a: dynamic, b: dynamic)
{
  return ((cpp_cast(a) * b) % mod);
}

func power(a: dynamic, b: dynamic)
{
  if ((!b))
  {
    return 1;
  }
  var u = power(a, (b >> 1));
  u = mul(u, u);
  if ((b & 1))
  {
    u = mul(u, a);
  }
  return u;
}

var rev: dynamic;

var roots: dynamic;

var mx = -1;

func init()
{
  mx = 16;
  roots.resize((1 << mx));
  {
    var j = 1;
    while ((j <= mx))
    {
      var mn = power(3, (((mod - 1)) >> j));
      {
        var i = 0;
        while ((i < ((1 << ((j - 1))))))
        {
          var npl = (((1 << ((j - 1)))) | i);
          if ((i == 0))
          {
            roots[npl] = 1;
          } else
          {
            roots[npl] = mul(mn, roots[(npl - 1)]);
          }
          i += 1;
        }
      }
      j += 1;
    }
  }
}

func calr(m: dynamic)
{
  rev.resize((1 << m));
  if ((mx == -1))
  {
    init();
  }
  rev[0] = 0;
  {
    var j = 0;
    while ((j < m))
    {
      {
        var k = 0;
        while ((k < ((1 << j))))
        {
          rev[(k | ((1 << j)))] = (rev[k] + ((1 << (((m - j) - 1)))));
          k += 1;
        }
      }
      j += 1;
    }
  }
}

func dft(a: dynamic)
{
  var n = a.size();
  var r = 0;
  while ((((1 << r)) < n))
  {
    r += 1;
  }
  calr(r);
  {
    var i = 0;
    while ((i < n))
    {
      if ((rev[i] > i))
      {
        swap(a[rev[i]], a[i]);
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < n))
        {
          {
            var k = 0;
            while ((k < i))
            {
              var mr = mul(a[((i + j) + k)], roots[(i + k)]);
              a[((i + j) + k)] = sub(a[(j + k)], mr);
              a[(j + k)] = add(a[(j + k)], mr);
              k += 1;
            }
          }
          j += ((i << 1));
        }
      }
      i <<= 1;
    }
  }
}

func mul1(a: dynamic, b: dynamic)
{
  var l = ((a.size() + b.size()) - 1);
  var r = 0;
  while ((((1 << r)) < l))
  {
    r += 1;
  }
  a.resize((1 << r));
  b.resize((1 << r));
  dft(a);
  dft(b);
  var bk = power((1 << r), (mod - 2));
  {
    var i = 0;
    while ((i < ((1 << r))))
    {
      a[i] = mul(mul(a[i], b[i]), bk);
      i += 1;
    }
  }
  reverse((a.begin() + 1), a.end());
  dft(a);
  a.resize(l);
  return a;
}

func mult(a: dynamic, b: dynamic)
{
  var dv = ((a.size() / 2));
  var fn = cpp_construct(((a.size() + b.size()) - 1));
  var u = cpp_array(2, 2);
  var t = [a, b];
  {
    var j = 0;
    while ((j < 2))
    {
      {
        var k = 0;
        while ((k < 2))
        {
          if ((k == 0))
          {
            u[j][k].resize(dv);
            {
              var s = 0;
              while ((s < dv))
              {
                u[j][k][s] = t[j][s];
                s += 1;
              }
            }
          } else
          {
            u[j][k].resize((t[j].size() - dv));
            {
              var s = dv;
              while ((s < t[j].size()))
              {
                u[j][k][(s - dv)] = t[j][s];
                s += 1;
              }
            }
          }
          k += 1;
        }
      }
      j += 1;
    }
  }
  {
    var j = 0;
    while ((j < 2))
    {
      {
        var k = 0;
        while ((k < 2))
        {
          var f1 = mul1(u[0][j], u[1][k]);
          {
            var m = 0;
            while ((m < f1.size()))
            {
              var id = (m + (((j + k)) * dv));
              fn[id] = add(fn[id], f1[m]);
              m += 1;
            }
          }
          k += 1;
        }
      }
      j += 1;
    }
  }
  return fn;
}

func otp(a: dynamic)
{
  {
    var i = 0;
    while ((i < a.size()))
    {
      printf("%d ", a[i]);
      i += 1;
    }
  }
  printf("\n");
}

var jc = cpp_array(maxn);

var bjc = cpp_array(maxn);

var b = cpp_array(maxn);

var a = cpp_array(maxn);

func main()
{
  var t: dynamic;
  read(t);
  jc[0] = cpp_assign(bjc[0], "=", 1);
  {
    var i = 1;
    while ((i < maxn))
    {
      jc[i] = ((jc[(i - 1)] * i) % mod);
      bjc[i] = ksm(jc[i], (mod - 2));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < t))
    {
      var n: dynamic;
      var m: dynamic;
      read(n, m);
      {
        var j = (n - 1);
        while ((j >= 0))
        {
          scanf("%d", (&b[j]));
          b[j] %= mod;
          if ((b[j] < 0))
          {
            b[j] += mod;
          }
          j -= 1;
        }
      }
      m %= mod;
      if ((!m))
      {
        {
          var j = 0;
          while ((j < n))
          {
            a[j] = b[j];
            j += 1;
          }
        }
      } else
      {
        if ((m < 0))
        {
          m += mod;
        }
        m = (mod - m);
        var bk = ksm(m, (mod - 2));
        {
          var j = 0;
          while ((j < n))
          {
            h[j] = bjc[j];
            g[j] = mul(mul(b[j], bjc[j]), ksm(bk, j));
            j += 1;
          }
        }
        var fn: dynamic;
        if ((n <= 20000))
        {
          fn = mul1(g, h);
        } else
        {
          fn = mult(g, h);
        }
        {
          var j = 0;
          while ((j < n))
          {
            a[j] = mul(mul(fn[j], ksm(m, j)), jc[j]);
            j += 1;
          }
        }
      }
      {
        var j = 0;
        while ((j < n))
        {
          printf("%d", a[j]);
          if ((j != (n - 1)))
          {
            printf(" ");
          } else
          {
            printf("\n");
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return 0;
}
