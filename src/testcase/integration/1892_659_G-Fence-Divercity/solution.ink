// Translated from solution.cpp.

var INF: dynamic = 0x3f3f3f3f;

var LINF: dynamic = 0x3f3f3f3f3f3f3f3f;

var MAX: dynamic = 1000011;

var n: dynamic = cpp_uninitialized();

var h: dynamic = cpp_array(MAX);

var MOD: dynamic = (int_cpp(1e9) + 7);

func add(a: dynamic, b: dynamic, mod: dynamic = MOD) -> dynamic
{
  return  ((((a + b) >= mod))) ? (((a + b) - mod)) : ((a + b));
}

func sub(a: dynamic, b: dynamic, mod: dynamic = MOD) -> dynamic
{
  return  ((((a - b) < 0))) ? (((a - b) + mod)) : ((a - b));
}

func inc(a: dynamic, b: dynamic, mod: dynamic = MOD) -> dynamic
{
  a = add(a, b, mod);
}

func negate(a: dynamic, mod: dynamic = MOD) -> dynamic
{
  return (mod - a);
}

func mul(a: dynamic, b: dynamic, mod: dynamic = MOD) -> dynamic
{
  return ((((a * 1) * b)) % mod);
}

func binPow(b: dynamic, p: dynamic, mod: dynamic = MOD) -> dynamic
{
  var r: dynamic = 1;
  while (p)
  {
    if ((p & 1))
    {
      r = mul(r, b, mod);
    }
    b = mul(b, b, mod);
    p >>= 1;
  }
  return r;
}

func inv(a: dynamic, mod: dynamic = MOD) -> dynamic
{
  var res: dynamic = binPow(a, (mod - 2), mod);
  return res;
}

func dvd(a: dynamic, b: dynamic, mod: dynamic = MOD) -> dynamic
{
  return mul(a, inv(b, mod), mod);
}

func clear() -> dynamic
{
}

func solve() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&h[i]));
      h[i] -= 1;
      i += 1;
    }
  }
  var ans: dynamic = 0;
  var f: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      ans = add(ans, h[i]);
      ans = add(ans, mul( (i) ? min(h[i], h[(i - 1)]) : h[i], f));
      if ((i < (n - 1)))
      {
        var mnr: dynamic = min(h[i], h[(i + 1)]);
        var mnl: dynamic =  (i) ? min(h[i], h[(i - 1)]) : h[i];
        var mn: dynamic = min(mnl, mnr);
        var nf: dynamic = mnr;
        nf = add(nf, mul(mn, f));
        f = nf;
      }
      i += 1;
    }
  }
  printf("%d\n", ans);
}

func main() -> dynamic
{
  while ((scanf("%d", (&n)) == 1))
  {
    clear();
    solve();
    return 0;
  }
  return 0;
}
