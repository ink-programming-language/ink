// Translated from solution.cpp.

var eps: dynamic = 1e-10;

var epsf: dynamic = 1e-6;

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if (((a == 0) || (b == 0)))
  {
    return max(a, b);
  }
  var tempa: dynamic = cpp_uninitialized();
  var tempb: dynamic = cpp_uninitialized();
  while (1)
  {
    if (((a % b) == 0))
    {
      return b;
    } else
    {
      tempa = a;
      tempb = b;
      a = tempb;
      b = (tempa % tempb);
    }
  }
}

func compfloat(x: dynamic, y: dynamic) -> dynamic
{
  if ((fabs((x - y)) < epsf))
  {
    return 0;
  } else if (((x - y) > 0))
  {
    return 1;
  }
  return -1;
}

func compdouble(x: dynamic, y: dynamic) -> dynamic
{
  if ((fabs((x - y)) < eps))
  {
    return 0;
  } else if (((x - y) > 0))
  {
    return 1;
  } else
  {
    return -1;
  }
}

func prime(k: dynamic) -> dynamic
{
  {
    var i: dynamic = 2;
    while (((i * i) <= k))
    {
      if (((k % i) == 0))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func pdash(n: dynamic = 1) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < 30))
        {
          write("-");
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
}

func cordinate_compression(v: dynamic) -> dynamic
{
  var p: dynamic = v;
  sort(p.begin(), p.end());
  p.erase(unique(p.begin(), p.end()), p.end());
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(((v).size()))))
    {
      v[i] = cpp_cast(((lower_bound(p.begin(), p.end(), v[i]) - p.begin())));
      i += 1;
    }
  }
}

var fact: dynamic = cpp_array(700006);

var ifact: dynamic = cpp_array(700006);

var mod: dynamic = 1000003;

func C(n: dynamic, m: dynamic) -> dynamic
{
  return (((fact[n] * ((((ifact[m] * ifact[(n - m)])) % mod)))) % mod);
}

func power(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  var result: dynamic = 1;
  x = (x % z);
  while ((y > 0))
  {
    if ((y & 1))
    {
      result = (((result * x)) % z);
    }
    y = (y >> 1);
    x = (((x * x)) % z);
  }
  return result;
}

func modInverse(n: dynamic, p: dynamic) -> dynamic
{
  return power(n, (p - 2), p);
}

func solve() -> dynamic
{
  fact[0] = cpp_assign(fact[1], "=", 1);
  {
    var i: dynamic = 2;
    while ((i <= 700005))
    {
      fact[i] = (((fact[(i - 1)] * i)) % mod);
      i += 1;
    }
  }
  ifact[700005] = (modInverse(fact[700005], mod) % mod);
  {
    var i: dynamic = 700004;
    while ((i >= 0))
    {
      ifact[i] = (((ifact[(i + 1)] * ((i + 1)))) % mod);
      i -= 1;
    }
  }
  var n: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  read(n, c);
  write((C((n + c), c) - 1), "\n");
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
