// Translated from solution.cpp.

var INF = 0x3f3f3f3f;

var LINF = 0x3f3f3f3f3f3f3f3f;

var MAX = 1000011;

var n: dynamic;

var h = cpp_array(MAX);

var MOD = (int_cpp(1e9) + 7);

func add(a: dynamic, b: dynamic, mod: dynamic = MOD)
{
  return if ((((a + b) >= mod))) (((a + b) - mod)) else ((a + b));
}

func sub(a: dynamic, b: dynamic, mod: dynamic = MOD)
{
  return if ((((a - b) < 0))) (((a - b) + mod)) else ((a - b));
}

func inc(a: dynamic, b: dynamic, mod: dynamic = MOD)
{
  a = add(a, b, mod);
}

func negate(a: dynamic, mod: dynamic = MOD)
{
  return (mod - a);
}

func mul(a: dynamic, b: dynamic, mod: dynamic = MOD)
{
  return ((((a * 1) * b)) % mod);
}

func binPow(b: dynamic, p: dynamic, mod: dynamic = MOD)
{
  var r = 1;
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

func inv(a: dynamic, mod: dynamic = MOD)
{
  var res = binPow(a, (mod - 2), mod);
  return res;
}

func dvd(a: dynamic, b: dynamic, mod: dynamic = MOD)
{
  return mul(a, inv(b, mod), mod);
}

func clear()
{
}

func solve()
{
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&h[i]));
      h[i] -= 1;
      i += 1;
    }
  }
  var ans = 0;
  var f = 0;
  {
    var i = 0;
    while ((i < n))
    {
      ans = add(ans, h[i]);
      ans = add(ans, mul(if (i) min(h[i], h[(i - 1)]) else h[i], f));
      if ((i < (n - 1)))
      {
        var mnr = min(h[i], h[(i + 1)]);
        var mnl = if (i) min(h[i], h[(i - 1)]) else h[i];
        var mn = min(mnl, mnr);
        var nf = mnr;
        nf = add(nf, mul(mn, f));
        f = nf;
      }
      i += 1;
    }
  }
  printf("%d\n", ans);
}

func main()
{
  while ((scanf("%d", (&n)) == 1))
  {
    clear();
    solve();
    return 0;
  }
  return 0;
}
