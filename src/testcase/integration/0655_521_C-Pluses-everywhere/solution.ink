// Translated from solution.cpp.

var MAXN = 100005;

var mod = (1e9 + 7);

var n: dynamic;

var k: dynamic;

var inv = cpp_array(MAXN);

var a = cpp_array(MAXN);

func pow(a: dynamic, b: dynamic)
{
  var res = 1;
  var tmp = a;
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

func solve()
{
  var sum = 0;
  var ans = 0;
  var ten = 1;
  var c = 1;
  {
    var i = (1);
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
      var i = (1);
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
    var i = ((n - k));
    while ((i <= ((n - 2))))
    {
      c = ((c * i) % mod);
      i += 1;
    }
  }
  {
    var i = (1);
    while ((i <= ((k - 1))))
    {
      c = ((c * inv[i]) % mod);
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < ((n - k))))
    {
      sum = ((((((sum - a[(n - i)])) % mod) + mod)) % mod);
      var tmpc = ((((c * inv[k]) % mod) * (((n - i) - 1))) % mod);
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

func main()
{
  {
    var i = (0);
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
