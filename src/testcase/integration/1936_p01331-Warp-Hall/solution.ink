// Translated from solution.cpp.

var MOD = 1000000007;

func add(a: dynamic, b: dynamic)
{
  return (((a + b)) % MOD);
}

func sub(a: dynamic, b: dynamic)
{
  return ((((a - b) + MOD)) % MOD);
}

func mul(a: dynamic, b: dynamic)
{
  return ((a * b) % MOD);
}

var f = cpp_array(200010);

func Inverse(a: dynamic, p: dynamic = MOD)
{
  if ((a == 1))
  {
    return 1;
  }
  return sub(0, mul((p / a), Inverse((p % a), p)));
}

func C(a: dynamic, b: dynamic)
{
  return mul(mul(f[a], Inverse(f[b])), Inverse(f[(a - b)]));
}

class Warp
{
  var sx: dynamic;
  var sy: dynamic;
  var tx: dynamic;
  var ty: dynamic;
  func operator_less(w: dynamic)
  {
      if ((sx != w.sx))
      {
        return (sx < w.sx);
      }
      return (sy < w.sy);
    }
}

var w = cpp_array(1010);

var d = cpp_array(1010);

var m: dynamic;

var n: dynamic;

var k: dynamic;

func Gao(sx: dynamic, sy: dynamic, tx: dynamic, ty: dynamic)
{
  if (((sx > tx) || (sy > ty)))
  {
    return 0;
  }
  return C((((tx - sx) + ty) - sy), (tx - sx));
}

func main()
{
  f[0] = 1;
  {
    var i = 1;
    while ((i <= 200000))
    {
      f[i] = mul(f[(i - 1)], i);
      i += 1;
    }
  }
  while (((scanf("%d%d%d", (&m), (&n), (&k)) != EOF) && (((m + n) + k) > 0)))
  {
    {
      var i = 0;
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
      var i = 0;
      while ((i <= k))
      {
        d[i] = C((w[i].sx + w[i].sy), w[i].sx);
        {
          var j = 0;
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
