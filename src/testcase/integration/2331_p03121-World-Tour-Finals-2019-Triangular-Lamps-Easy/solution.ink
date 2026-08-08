// Translated from solution.cpp.

var mod = 1000000007;

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
  if ((UF[a] > UF[b]))
  {
    swap(a, b);
  }
  UF[a] += UF[b];
  UF[b] = a;
}

var x = cpp_array(110000);

var y = cpp_array(110000);

func main()
{
  var a: dynamic;
  scanf("%d", (&a));
  {
    var i = 0;
    while ((i < a))
    {
      scanf("%lld%lld", (x + i), (y + i));
      i += 1;
    }
  }
  var D = 1000000000000000000;
  var ret = 0;
  {
    var i = 0;
    while ((i < 61))
    {
      var cnt = 0;
      var at = ((1 << i));
      {
        var j = 0;
        while ((j < a))
        {
          var n = (D - ((x[j] + y[j])));
          var k = (at - y[j]);
          if (((n < k) || (k < 0)))
          {
            j += 1;
            continue;
          }
          if ((((n & k)) == k))
          {
            cnt += 1;
          }
          j += 1;
        }
      }
      if ((cnt % 2))
      {
        ret += ((1 << i));
      }
      i += 1;
    }
  }
  printf("%lld\n", (D - ret));
}
