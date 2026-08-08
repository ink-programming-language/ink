// Translated from solution.cpp.

var mod = (1e9 + 7);

var fact = cpp_array(1000001);

var inv = cpp_array(1000001);

var primes = cpp_array(100007);

var arr = cpp_array(1000007);

func modPower(b: dynamic, p: dynamic)
{
  if ((p == 0))
  {
    return 1;
  }
  var halfpow = modPower(b, (p / 2));
  var toReturn = (((halfpow * halfpow)) % mod);
  if ((p % 2))
  {
    toReturn = (((toReturn * b)) % mod);
  }
  return toReturn;
}

func fastPower(b: dynamic, p: dynamic)
{
  if ((p == 0))
  {
    return 1;
  }
  var ans = fastPower(b, (p / 2));
  ans = ((ans * ans));
  if (((p % 2) != 0))
  {
    ans = ((ans * b));
  }
  return ans;
}

func GcdRecursive(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return a;
  }
  return GcdRecursive(b, (a % b));
}

func modLCM(a: dynamic, b: dynamic)
{
  var val = GcdRecursive(a, b);
  var tmp = (((((a % mod)) * ((b % mod)))) % mod);
  var finalVal = (((((tmp % mod)) * ((arr[val] % mod)))) % mod);
  return finalVal;
}

func LCM(a: dynamic, b: dynamic)
{
  return (((a * b)) / GcdRecursive(a, b));
}

func move1step(a: dynamic, b: dynamic, q: dynamic)
{
  var c = (a - (q * b));
  a = b;
  b = c;
}

func GcdIterative(a: dynamic, b: dynamic)
{
  while (b)
  {
    move1step(a, b, (a / b));
  }
  return a;
}

func pre(n: dynamic)
{
  fact[0] = 1;
  inv[0] = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      fact[i] = (((i * fact[(i - 1)])) % mod);
      inv[i] = modPower(fact[i], (mod - 2));
      arr[i] = modPower(i, (mod - 2));
      i += 1;
    }
  }
}

func npr(n: dynamic, r: dynamic)
{
  return ((((fact[n] * inv[(n - r)])) % mod));
}

func ncr(n: dynamic, r: dynamic)
{
  return ((((((((fact[n] * inv[(n - r)])) % mod)) * inv[r])) % mod));
}

func sieve(val: dynamic)
{
  memset(primes, 1, cpp_sizeof(primes));
  primes[0] = cpp_assign(primes[1], "=", false);
  {
    var i = 2;
    while ((i <= val))
    {
      if (primes[i])
      {
        {
          var j = (i * i);
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

func dot(a: dynamic, b: dynamic)
{
  return ((conj(a) * b)).real();
}

func cross(a: dynamic, b: dynamic)
{
  return ((conj(a) * b)).imag();
}

func angleBetVec(a: dynamic, b: dynamic)
{
  var d = dot(a, b);
  d /= abs(a);
  d /= abs(b);
  return ((acos(d) * 180) / acos(-1));
}

func RotateAbout(a: dynamic, about: dynamic, angle: dynamic)
{
  return ((((a - about)) * polar(cpp_cast(1.0), angle)) + about);
}

var MOD = 998244353;

var N = (2e5 + 7);

var inf = (1e18 + 5);

var t: dynamic;

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  ios_base.sync_with_stdio(0);
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var m: dynamic;
    read(n, m);
    if ((m == 0))
    {
      write(0, "\n");
      continue;
    }
    var ans = ((n * ((n + 1))) / 2);
    var diff = (n - m);
    var zeroSubSeq = floor((diff / ((m + 1))));
    ans -= (((zeroSubSeq * ((zeroSubSeq + 1))) / 2) * ((m + 1)));
    ans -= (((zeroSubSeq + 1)) * ((diff % ((m + 1)))));
    write(ans, "\n");
  }
  return 0;
}
