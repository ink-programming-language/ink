// Translated from solution.cpp.

var kMod = 998244353;

var kN = int_cpp((2E5 + 10));

class BIT
{
  var val: dynamic = cpp_array(kN);
  func init()
  {
      memset(val, 0, cpp_sizeof((val)));
    }
  func add(pos: dynamic, x: dynamic)
  {
      while ((pos < kN))
      {
        val[pos] = (((val[pos] + x)) % kMod);
        pos += (pos & (-pos));
      }
      return;
    }
  func ask(pos: dynamic)
  {
      var ans = 0;
      while (pos)
      {
        ans += val[pos];
        pos ^= (pos & (-pos));
      }
      return (ans % kMod);
    }
}

func Pow(a: dynamic, b: dynamic)
{
  var ans = 1;
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

func Rev(n: dynamic)
{
  return Pow(n, (kMod - 2));
}

var a = cpp_array(kN);

var f = cpp_array(kN);

var p = cpp_array(kN);

var bit: dynamic;

var bcnt: dynamic;

func main()
{
  var n: dynamic;
  var k: dynamic;
  var ans = 0;
  var tot = 0;
  var sum = 0;
  scanf("%d%d", (&n), (&k));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  p[0] = 1;
  p[1] = ((((k - 1)) * Rev(k)) % kMod);
  {
    var i = 2;
    while ((i <= n))
    {
      p[i] = ((p[(i - 1)] * p[1]) % kMod);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= k))
    {
      f[i] = 0;
      i += 1;
    }
  }
  {
    var i = (k + 1);
    while ((i <= n))
    {
      f[i] = (i - k);
      i += 1;
    }
  }
  bit.init();
  bcnt.init();
  {
    var i = 1;
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
