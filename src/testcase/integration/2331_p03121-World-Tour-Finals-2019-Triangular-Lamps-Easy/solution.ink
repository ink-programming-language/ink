// Translated from solution.cpp.

var mod: dynamic = 1000000007;

var inf: dynamic = (mod * mod);

var d2: dynamic = (((mod + 1)) / 2);

var EPS: dynamic = 1e-9;

var INF: dynamic = 1e+10;

var PI: dynamic = acos(-1.0);

var C_SIZE: dynamic = 3100000;

var UF_SIZE: dynamic = 3100000;

var fact: dynamic = cpp_array(C_SIZE);

var finv: dynamic = cpp_array(C_SIZE);

var inv: dynamic = cpp_array(C_SIZE);

func Comb(a: dynamic, b: dynamic) -> dynamic
{
  if (((a < b) || (b < 0)))
  {
    return 0;
  }
  return ((((fact[a] * finv[b]) % mod) * finv[(a - b)]) % mod);
}

func init_C(n: dynamic) -> dynamic
{
  fact[0] = cpp_assign(finv[0], "=", cpp_assign(inv[1], "=", 1));
  {
    var i: dynamic = 2;
    while ((i < n))
    {
      inv[i] = (((mod - ((((mod / i)) * inv[(mod % i)]) % mod))) % mod);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      fact[i] = ((fact[(i - 1)] * i) % mod);
      finv[i] = ((finv[(i - 1)] * inv[i]) % mod);
      i += 1;
    }
  }
}

func pw(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < 0))
  {
    return 0;
  }
  if ((b < 0))
  {
    return 0;
  }
  var ret: dynamic = 1;
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

func pw_mod(a: dynamic, b: dynamic, M: dynamic) -> dynamic
{
  if ((a < 0))
  {
    return 0;
  }
  if ((b < 0))
  {
    return 0;
  }
  var ret: dynamic = 1;
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

func pw_mod_int(a: dynamic, b: dynamic, M: dynamic) -> dynamic
{
  if ((a < 0))
  {
    return 0;
  }
  if ((b < 0))
  {
    return 0;
  }
  var ret: dynamic = 1;
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

func ABS(a: dynamic) -> dynamic
{
  return max(a, (-a));
}

func ABS(a: dynamic) -> dynamic
{
  return max(a, (-a));
}

func ABS(a: dynamic) -> dynamic
{
  return max(a, (-a));
}

func sig(r: dynamic) -> dynamic
{
  return  (((r < (-EPS)))) ? -1 :  (((r > (+EPS)))) ? +1 : 0;
}

var UF: dynamic = cpp_array(UF_SIZE);

func init_UF(n: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      UF[i] = -1;
      i += 1;
    }
  }
}

func FIND(a: dynamic) -> dynamic
{
  if ((UF[a] < 0))
  {
    return a;
  }
  return cpp_assign(UF[a], "=", FIND(UF[a]));
}

func UNION(a: dynamic, b: dynamic) -> dynamic
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

var x: dynamic = cpp_array(110000);

var y: dynamic = cpp_array(110000);

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  scanf("%d", (&a));
  {
    var i: dynamic = 0;
    while ((i < a))
    {
      scanf("%lld%lld", (x + i), (y + i));
      i += 1;
    }
  }
  var D: dynamic = 1000000000000000000;
  var ret: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 61))
    {
      var cnt: dynamic = 0;
      var at: dynamic = ((1 << i));
      {
        var j: dynamic = 0;
        while ((j < a))
        {
          var n: dynamic = (D - ((x[j] + y[j])));
          var k: dynamic = (at - y[j]);
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
