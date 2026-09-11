// Translated from solution.cpp.

var MAXN: dynamic = 100005;

var mod: dynamic = (1e9 + 7);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var inv: dynamic = cpp_array(MAXN);

var a: dynamic = cpp_array(MAXN);

func pow(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = 1;
  var tmp: dynamic = a;
  while (b)
  {
    if ((b & 1))
    {
      res = ((res * tmp) % mod);
    }
    tmp = ((tmp * tmp) % mod);
    b >>= 1;
  }
  return res;
}

func solve() -> dynamic
{
  var sum: dynamic = 0;
  var ans: dynamic = 0;
  var ten: dynamic = 1;
  var c: dynamic = 1;
  {
    var i: dynamic = (1);
    while ((i <= (n)))
    {
      scanf("%1d", (&a[i]));
      sum += a[i];
      i += 1;
    }
  }
  if ((k == 0))
  {
    {
      var i: dynamic = (1);
      while ((i <= (n)))
      {
        ans = ((((ans * 10) + a[i])) % mod);
        i += 1;
      }
    }
    printf("%I64d\n", ans);
    return;
  }
  {
    var i: dynamic = ((n - k));
    while ((i <= ((n - 2))))
    {
      c = ((c * i) % mod);
      i += 1;
    }
  }
  {
    var i: dynamic = (1);
    while ((i <= ((k - 1))))
    {
      c = ((c * inv[i]) % mod);
      i += 1;
    }
  }
  {
    var i: dynamic = (0);
    while ((i < ((n - k))))
    {
      sum = ((((((sum - a[(n - i)])) % mod) + mod)) % mod);
      var tmpc: dynamic = ((((c * inv[k]) % mod) * (((n - i) - 1))) % mod);
      ans = (((ans + (((ten * sum) % mod) * c))) % mod);
      ans = (((ans + (((ten * a[(n - i)]) % mod) * tmpc))) % mod);
      ten = ((ten * 10) % mod);
      c = ((c * ((((n - k) - i) - 1))) % mod);
      c = ((c * inv[((n - i) - 2)]) % mod);
      i += 1;
    }
  }
  printf("%I64d\n", ans);
}

func main() -> dynamic
{
  {
    var i: dynamic = (0);
    while ((i < (MAXN)))
    {
      inv[i] = pow(i, (mod - 2));
      i += 1;
    }
  }
  while ((~scanf("%d%d", (&n), (&k))))
  {
    solve();
  }
  return 0;
}
