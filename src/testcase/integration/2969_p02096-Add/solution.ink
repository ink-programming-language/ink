// Translated from solution.cpp.

var ll: dynamic = dynamic;

var maxn: dynamic = cpp_expression("#inclu");

var mod: dynamic = cpp_expression("#incl");

var db: dynamic = cpp_expression("#inclu");

var vi: dynamic = cpp_expression("#include <b");

var pb: dynamic = cpp_expression("#include");

var mp: dynamic = cpp_expression("#include");

var pi: dynamic = cpp_expression("#include <bits");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

func ksm(a: dynamic, b: dynamic) -> dynamic
{
  if ((!b))
  {
    return 1;
  }
  var ns: dynamic = ksm(a, (b >> 1));
  ns = ((ns * ns) % mod);
  if ((b & 1))
  {
    ns = ((ns * a) % mod);
  }
  return ns;
}

func add(a: dynamic, b: dynamic) -> dynamic
{
  a += b;
  return  (((a < mod))) ? a : ((a - mod));
}

func sub(a: dynamic, b: dynamic) -> dynamic
{
  a -= b;
  return  (((a < 0))) ? (a + mod) : a;
}

func mul(a: dynamic, b: dynamic) -> dynamic
{
  return ((cpp_cast(a) * b) % mod);
}

func power(a: dynamic, b: dynamic) -> dynamic
{
  if ((!b))
  {
    return 1;
  }
  var u: dynamic = power(a, (b >> 1));
  u = mul(u, u);
  if ((b & 1))
  {
    u = mul(u, a);
  }
  return u;
}

var rev: dynamic = cpp_uninitialized();

var roots: dynamic = cpp_uninitialized();

var mx: dynamic = -1;

func init() -> dynamic
{
  mx = 16;
  roots.resize((1 << mx));
  {
    var j: dynamic = 1;
    while ((j <= mx))
    {
      var mn: dynamic = power(3, (((mod - 1)) >> j));
      {
        var i: dynamic = 0;
        while ((i < ((1 << ((j - 1))))))
        {
          var npl: dynamic = (((1 << ((j - 1)))) | i);
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

func calr(m: dynamic) -> dynamic
{
  rev.resize((1 << m));
  if ((mx == -1))
  {
    init();
  }
  rev[0] = 0;
  {
    var j: dynamic = 0;
    while ((j < m))
    {
      {
        var k: dynamic = 0;
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

func dft(a: dynamic) -> dynamic
{
  var n: dynamic = a.size();
  var r: dynamic = 0;
  while ((((1 << r)) < n))
  {
    r += 1;
  }
  calr(r);
  {
    var i: dynamic = 0;
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
    var i: dynamic = 1;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          {
            var k: dynamic = 0;
            while ((k < i))
            {
              var mr: dynamic = mul(a[((i + j) + k)], roots[(i + k)]);
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

func mul1(a: dynamic, b: dynamic) -> dynamic
{
  var l: dynamic = ((a.size() + b.size()) - 1);
  var r: dynamic = 0;
  while ((((1 << r)) < l))
  {
    r += 1;
  }
  a.resize((1 << r));
  b.resize((1 << r));
  dft(a);
  dft(b);
  var bk: dynamic = power((1 << r), (mod - 2));
  {
    var i: dynamic = 0;
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

func mult(a: dynamic, b: dynamic) -> dynamic
{
  var dv: dynamic = ((a.size() / 2));
  var fn: dynamic = cpp_construct(((a.size() + b.size()) - 1));
  var u: dynamic = cpp_array(2, 2);
  var t: dynamic = [a, b];
  {
    var j: dynamic = 0;
    while ((j < 2))
    {
      {
        var k: dynamic = 0;
        while ((k < 2))
        {
          if ((k == 0))
          {
            u[j][k].resize(dv);
            {
              var s: dynamic = 0;
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
              var s: dynamic = dv;
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
    var j: dynamic = 0;
    while ((j < 2))
    {
      {
        var k: dynamic = 0;
        while ((k < 2))
        {
          var f1: dynamic = mul1(u[0][j], u[1][k]);
          {
            var m: dynamic = 0;
            while ((m < f1.size()))
            {
              var id: dynamic = (m + (((j + k)) * dv));
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

func otp(a: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < a.size()))
    {
      printf("%d ", a[i]);
      i += 1;
    }
  }
  printf("\n");
}

var jc: dynamic = cpp_array(maxn);

var bjc: dynamic = cpp_array(maxn);

var b: dynamic = cpp_array(maxn);

var a: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  jc[0] = cpp_assign(bjc[0], "=", 1);
  {
    var i: dynamic = 1;
    while ((i < maxn))
    {
      jc[i] = ((jc[(i - 1)] * i) % mod);
      bjc[i] = ksm(jc[i], (mod - 2));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < t))
    {
      var n: dynamic = cpp_uninitialized();
      var m: dynamic = cpp_uninitialized();
      read(n, m);
      {
        var j: dynamic = (n - 1);
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
          var j: dynamic = 0;
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
        var bk: dynamic = ksm(m, (mod - 2));
        {
          var j: dynamic = 0;
          while ((j < n))
          {
            h[j] = bjc[j];
            g[j] = mul(mul(b[j], bjc[j]), ksm(bk, j));
            j += 1;
          }
        }
        var fn: dynamic = cpp_uninitialized();
        if ((n <= 20000))
        {
          fn = mul1(g, h);
        } else
        {
          fn = mult(g, h);
        }
        {
          var j: dynamic = 0;
          while ((j < n))
          {
            a[j] = mul(mul(fn[j], ksm(m, j)), jc[j]);
            j += 1;
          }
        }
      }
      {
        var j: dynamic = 0;
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
