// Translated from solution.cpp.

var kMod: dynamic = 998244353;

var kN: dynamic = int_cpp((2E5 + 10));

class BIT
{
  var val: dynamic = cpp_array(kN);
  func init() -> dynamic
  {
      memset(val, 0, cpp_sizeof((val)));
    }
  func add(pos: dynamic, x: dynamic) -> dynamic
  {
      while ((pos < kN))
      {
        val[pos] = (((val[pos] + x)) % kMod);
        pos += (pos & (-pos));
      }
      return;
    }
  func ask(pos: dynamic) -> dynamic
  {
      var ans: dynamic = 0;
      while (pos)
      {
        ans += val[pos];
        pos ^= (pos & (-pos));
      }
      return (ans % kMod);
    }
}

func Pow(a: dynamic, b: dynamic) -> dynamic
{
  var ans: dynamic = 1;
  while (b)
  {
    if ((b & 1))
    {
      ans = ((ans * a) % kMod);
    }
    a = ((a * a) % kMod);
    b >>= 1;
  }
  return ans;
}

func Rev(n: dynamic) -> dynamic
{
  return Pow(n, (kMod - 2));
}

var a: dynamic = cpp_array(kN);

var f: dynamic = cpp_array(kN);

var p: dynamic = cpp_array(kN);

var bit: dynamic = cpp_uninitialized();

var bcnt: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  var tot: dynamic = 0;
  var sum: dynamic = 0;
  scanf("%d%d", (&n), (&k));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  p[0] = 1;
  p[1] = ((((k - 1)) * Rev(k)) % kMod);
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      p[i] = ((p[(i - 1)] * p[1]) % kMod);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      f[i] = 0;
      i += 1;
    }
  }
  {
    var i: dynamic = (k + 1);
    while ((i <= n))
    {
      f[i] = (i - k);
      i += 1;
    }
  }
  bit.init();
  bcnt.init();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      ans += ((p[f[i]] * tot) % kMod);
      ans -= ((p[f[i]] * (((tot + tot) - bit.ask(a[i])))) % kMod);
      ans += (((i - 1) - bcnt.ask(a[i])));
      bit.add(a[i], Rev(p[f[i]]));
      bcnt.add(a[i], 1);
      tot = (((tot + (Rev(p[f[i]]) * Rev(2)))) % kMod);
      i += 1;
    }
  }
  ans %= kMod;
  if ((ans < 0))
  {
    ans += kMod;
  }
  printf("%lld\n", ans);
}
