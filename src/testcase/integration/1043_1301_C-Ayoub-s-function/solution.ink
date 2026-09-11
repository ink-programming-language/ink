// Translated from solution.cpp.

var mod: dynamic = (1e9 + 7);

var fact: dynamic = cpp_array(1000001);

var inv: dynamic = cpp_array(1000001);

var primes: dynamic = cpp_array(100007);

var arr: dynamic = cpp_array(1000007);

func modPower(b: dynamic, p: dynamic) -> dynamic
{
  if ((p == 0))
  {
    return 1;
  }
  var halfpow: dynamic = modPower(b, (p / 2));
  var toReturn: dynamic = (((halfpow * halfpow)) % mod);
  if ((p % 2))
  {
    toReturn = (((toReturn * b)) % mod);
  }
  return toReturn;
}

func fastPower(b: dynamic, p: dynamic) -> dynamic
{
  if ((p == 0))
  {
    return 1;
  }
  var ans: dynamic = fastPower(b, (p / 2));
  ans = ((ans * ans));
  if (((p % 2) != 0))
  {
    ans = ((ans * b));
  }
  return ans;
}

func GcdRecursive(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return a;
  }
  return GcdRecursive(b, (a % b));
}

func modLCM(a: dynamic, b: dynamic) -> dynamic
{
  var val: dynamic = GcdRecursive(a, b);
  var tmp: dynamic = (((((a % mod)) * ((b % mod)))) % mod);
  var finalVal: dynamic = (((((tmp % mod)) * ((arr[val] % mod)))) % mod);
  return finalVal;
}

func LCM(a: dynamic, b: dynamic) -> dynamic
{
  return (((a * b)) / GcdRecursive(a, b));
}

func move1step(a: dynamic, b: dynamic, q: dynamic) -> dynamic
{
  var c: dynamic = (a - (q * b));
  a = b;
  b = c;
}

func GcdIterative(a: dynamic, b: dynamic) -> dynamic
{
  while (b)
  {
    move1step(a, b, (a / b));
  }
  return a;
}

func pre(n: dynamic) -> dynamic
{
  fact[0] = 1;
  inv[0] = 1;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      fact[i] = (((i * fact[(i - 1)])) % mod);
      inv[i] = modPower(fact[i], (mod - 2));
      arr[i] = modPower(i, (mod - 2));
      i += 1;
    }
  }
}

func npr(n: dynamic, r: dynamic) -> dynamic
{
  return ((((fact[n] * inv[(n - r)])) % mod));
}

func ncr(n: dynamic, r: dynamic) -> dynamic
{
  return ((((((((fact[n] * inv[(n - r)])) % mod)) * inv[r])) % mod));
}

func sieve(val: dynamic) -> dynamic
{
  memset(primes, 1, cpp_sizeof(primes));
  primes[0] = cpp_assign(primes[1], "=", false);
  {
    var i: dynamic = 2;
    while ((i <= val))
    {
      if (primes[i])
      {
        {
          var j: dynamic = (i * i);
          while ((j <= val))
          {
            primes[j] = 0;
            j += i;
          }
        }
      }
      i += 1;
    }
  }
}

func dot(a: dynamic, b: dynamic) -> dynamic
{
  return ((conj(a) * b)).real();
}

func cross(a: dynamic, b: dynamic) -> dynamic
{
  return ((conj(a) * b)).imag();
}

func angleBetVec(a: dynamic, b: dynamic) -> dynamic
{
  var d: dynamic = dot(a, b);
  d /= abs(a);
  d /= abs(b);
  return ((acos(d) * 180) / acos(-1));
}

func RotateAbout(a: dynamic, about: dynamic, angle: dynamic) -> dynamic
{
  return ((((a - about)) * polar(cpp_cast(1.0), angle)) + about);
}

var MOD: dynamic = 998244353;

var N: dynamic = (2e5 + 7);

var inf: dynamic = (1e18 + 5);

var t: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  ios_base.sync_with_stdio(0);
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    read(n, m);
    if ((m == 0))
    {
      write(0, "\n");
      continue;
    }
    var ans: dynamic = ((n * ((n + 1))) / 2);
    var diff: dynamic = (n - m);
    var zeroSubSeq: dynamic = floor((diff / ((m + 1))));
    ans -= (((zeroSubSeq * ((zeroSubSeq + 1))) / 2) * ((m + 1)));
    ans -= (((zeroSubSeq + 1)) * ((diff % ((m + 1)))));
    write(ans, "\n");
  }
  return 0;
}
