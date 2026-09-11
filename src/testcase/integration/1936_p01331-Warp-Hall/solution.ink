// Translated from solution.cpp.

var MOD: dynamic = 1000000007;

func add(a: dynamic, b: dynamic) -> dynamic
{
  return (((a + b)) % MOD);
}

func sub(a: dynamic, b: dynamic) -> dynamic
{
  return ((((a - b) + MOD)) % MOD);
}

func mul(a: dynamic, b: dynamic) -> dynamic
{
  return ((a * b) % MOD);
}

var f: dynamic = cpp_array(200010);

func Inverse(a: dynamic, p: dynamic = MOD) -> dynamic
{
  if ((a == 1))
  {
    return 1;
  }
  return sub(0, mul((p / a), Inverse((p % a), p)));
}

func C(a: dynamic, b: dynamic) -> dynamic
{
  return mul(mul(f[a], Inverse(f[b])), Inverse(f[(a - b)]));
}

class Warp
{
  var sx: dynamic = cpp_uninitialized();
  var sy: dynamic = cpp_uninitialized();
  var tx: dynamic = cpp_uninitialized();
  var ty: dynamic = cpp_uninitialized();
  func operator_less(w: dynamic) -> dynamic
  {
      if ((sx != w.sx))
      {
        return (sx < w.sx);
      }
      return (sy < w.sy);
    }
}

var w: dynamic = cpp_array(1010);

var d: dynamic = cpp_array(1010);

var m: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

func Gao(sx: dynamic, sy: dynamic, tx: dynamic, ty: dynamic) -> dynamic
{
  if (((sx > tx) || (sy > ty)))
  {
    return 0;
  }
  return C((((tx - sx) + ty) - sy), (tx - sx));
}

func main() -> dynamic
{
  f[0] = 1;
  {
    var i: dynamic = 1;
    while ((i <= 200000))
    {
      f[i] = mul(f[(i - 1)], i);
      i += 1;
    }
  }
  while (((scanf("%d%d%d", (&m), (&n), (&k)) != EOF) && (((m + n) + k) > 0)))
  {
    {
      var i: dynamic = 0;
      while ((i < k))
      {
        scanf("%d%d%d%d", (&w[i].sx), (&w[i].sy), (&w[i].tx), (&w[i].ty));
        w[i].sx -= 1;
        w[i].sy -= 1;
        w[i].tx -= 1;
        w[i].ty -= 1;
        i += 1;
      }
    }
    memset(d, 0, cpp_sizeof((d)));
    sort(w, (w + k));
    w[k].sx = (m - 1);
    w[k].sy = (n - 1);
    {
      var i: dynamic = 0;
      while ((i <= k))
      {
        d[i] = C((w[i].sx + w[i].sy), w[i].sx);
        {
          var j: dynamic = 0;
          while ((j < i))
          {
            d[i] = add(d[i], mul(d[j], sub(Gao(w[j].tx, w[j].ty, w[i].sx, w[i].sy), Gao(w[j].sx, w[j].sy, w[i].sx, w[i].sy))));
            j += 1;
          }
        }
        i += 1;
      }
    }
    printf("%lld\n", d[k]);
  }
  return 0;
}
