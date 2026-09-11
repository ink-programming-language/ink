// Translated from solution.cpp.

func all(a: dynamic) -> dynamic
{
  return cpp_expression("#include <iostrea");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (ll i = 0; i < (n); i++)");
}

var pb: dynamic = cpp_expression("#include");

func debug(x: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream> #include <vector> #include <a");
}

var inf: dynamic = 1000000010;

var INF: dynamic = 1000000000000000010;

var eps: dynamic = 1e-12;

var pi: dynamic = 3.141592653589793238;

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
    return true;
  }
  return false;
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
    return true;
  }
  return false;
}

var mod: dynamic = 1000000007;

var fac: dynamic = cpp_uninitialized();

var inv: dynamic = cpp_uninitialized();

var facinv: dynamic = cpp_uninitialized();

func modcalc(n: dynamic) -> dynamic
{
  fac.resize(n);
  inv.resize(n);
  facinv.resize(n);
  fac[0] = 1;
  fac[1] = 1;
  inv[1] = 1;
  facinv[0] = 1;
  facinv[1] = 1;
  {
    var i: dynamic = 2;
    while ((i < n))
    {
      fac[i] = ((fac[(i - 1)] * i) % mod);
      inv[i] = (mod - ((inv[(mod % i)] * ((mod / i))) % mod));
      facinv[i] = ((facinv[(i - 1)] * inv[i]) % mod);
      i += 1;
    }
  }
}

func modinv(a: dynamic) -> dynamic
{
  a %= mod;
  if ((a == 0))
  {
    abort();
  }
  if ((a < cpp_cast(inv.size())))
  {
    return inv[a];
  }
  var b: dynamic = mod;
  var u: dynamic = 1;
  var v: dynamic = 0;
  while (b)
  {
    var t: dynamic = (a / b);
    a -= (t * b);
    swap(a, b);
    u -= (t * v);
    swap(u, v);
  }
  u %= mod;
  if ((u < 0))
  {
    u += mod;
  }
  return u;
}

func modpow(a: dynamic, b: dynamic, m: dynamic = mod) -> dynamic
{
  var ans: dynamic = 1;
  a %= m;
  while (b)
  {
    if ((b & 1))
    {
      ans = ((ans * a) % m);
    }
    a = ((a * a) % m);
    b >>= 1;
  }
  return ans;
}

func modcomb(n: dynamic, k: dynamic) -> dynamic
{
  if ((((n < 0) || (k < 0)) || (n < k)))
  {
    return 0;
  }
  return ((((fac[n] * facinv[k]) % mod) * facinv[(n - k)]) % mod);
}

func modperm(n: dynamic, k: dynamic) -> dynamic
{
  if ((((n < 0) || (k < 0)) || (n < k)))
  {
    return 0;
  }
  return ((fac[n] * facinv[(n - k)]) % mod);
}

func modhom(n: dynamic, k: dynamic) -> dynamic
{
  if ((((n < 0) || (k < 0)) || ((n == 0) && (k > 0))))
  {
    return 0;
  }
  if (((n == 0) && (k == 0)))
  {
    return 1;
  }
  return ((((fac[((n + k) - 1)] * facinv[k]) % mod) * facinv[(n - 1)]) % mod);
}

class segtree
{
  var n: dynamic = cpp_uninitialized();
  var data: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
  func operation(a: dynamic, b: dynamic) -> dynamic
  {
      return (a + b);
    }
  func segtree(n: dynamic) -> dynamic
  {
      n = 1;
      while ((n < (n + 2)))
      {
        n <<= 1;
      }
      data = vector((2 * n), id);
    }
  func change(i: dynamic, x: dynamic) -> dynamic
  {
      i += n;
      data[i] = x;
      while ((i > 1))
      {
        i >>= 1;
        data[i] = operation(data[(i << 1)], data[((i << 1) | 1)]);
      }
    }
  func add(i: dynamic, x: dynamic) -> dynamic
  {
      change(i, (data[(i + n)] + x));
    }
  func get(a: dynamic, b: dynamic) -> dynamic
  {
      var left: dynamic = id;
      var right: dynamic = id;
      a += n;
      b += n;
      while ((a < b))
      {
        if ((a & 1))
        {
          left = operation(left, data[cpp_update(a, "++")]);
        }
        if ((b & 1))
        {
          right = operation(data[cpp_update(b, "--")], right);
        }
        a >>= 1;
        b >>= 1;
      }
      return operation(left, right);
    }
  func get_all() -> dynamic
  {
      return data[1];
    }
  func operator_index(i: dynamic) -> dynamic
  {
      return data[(i + n)];
    }
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  write(fixed, setprecision(20));
  modcalc(500010);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var vec: dynamic = cpp_uninitialized();
  rep(i, n);
  if ((!ok[i]))
  {
    vec.pb(i);
  }
  var vs: dynamic = vec.size();
  reverse(all(a));
  var ans: dynamic = 0;
  var d: dynamic = 0;
  var zero: dynamic = 0;
  ans %= mod;
  ans += 1;
  ans *= fac[vs];
  ans %= mod;
  write(ans, cpp_char("\n"));
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    read(a[i]);
    a[i] -= 1;
    if ((a[i] != -1))
    {
      ok[a[i]] = true;
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var p: dynamic = 0;
    if ((a[i] == -1))
    {
      p += ((zero * modinv(2)) % mod);
      p += d;
      p %= mod;
      zero += 1;
    } else
    {
      if (vs)
      {
        p += ((zero * modinv(vs)) % mod);
        var idx: dynamic = (upper_bound(all(vec), a[i]) - vec.begin());
        p *= idx;
        p %= mod;
        d += ((mod + 1) - ((idx * modinv(vs)) % mod));
        d %= mod;
      }
      p += seg.get(0, a[i]);
      seg.add(a[i], 1);
    }
    ans += ((p * fac[i]) % mod);
  }
