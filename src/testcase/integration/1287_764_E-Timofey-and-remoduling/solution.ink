// Translated from solution.cpp.

var a = cpp_array(300000);

var s = cpp_array(2);

var b = cpp_array(300000);

func fp(a: dynamic, k: dynamic, m: dynamic)
{
  var res = 1;
  while (k)
  {
    if ((k & 1))
    {
      res = (((1 * res) * a) % m);
    }
    a = (((1 * a) * a) % m);
    k >>= 1;
  }
  return res;
}

func main()
{
  var m: dynamic;
  var n: dynamic;
  scanf("%d%d", (&m), (&n));
  s[0] = cpp_assign(s[1], "=", 0);
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      s[0] = (((s[0] + a[i])) % m);
      s[1] = (((s[1] + ((1 * a[i]) * a[i]))) % m);
      i += 1;
    }
  }
  if ((n == 1))
  {
    return (0 * printf("%d 0\n", a[1]));
  }
  sort((a + 1), ((a + n) + 1));
  {
    var i = 1;
    while ((i <= n))
    {
      if ((i == 2))
      {
        i += 1;
        continue;
      }
      var d = (((abs((a[i] - a[2])) + m)) % m);
      var x = ((((1 * (((s[0] - ((((((1 * n) * ((n - 1))) / 2) % m) * d) % m)) + m))) % m) * fp(n, (m - 2), m)) % m);
      var ans = (((((1 * n) * x) % m) * x) % m);
      ans = (((ans + (((((((1 * n) * ((n - 1))) % m) * d) % m) * x) % m))) % m);
      ans = (((ans + (((((((((1 * n) * ((n - 1))) * (((2 * n) - 1))) / 6) % m) * d) % m) * d) % m))) % m);
      if ((ans == s[1]))
      {
        b[1] = x;
        {
          var j = 2;
          while ((j <= n))
          {
            b[j] = (b[(j - 1)] + d);
            b[j] %= m;
            j += 1;
          }
        }
        sort((b + 1), ((b + n) + 1));
        var flag = true;
        {
          var j = 1;
          while ((j <= n))
          {
            if ((a[j] != b[j]))
            {
              flag = false;
              break;
            }
            j += 1;
          }
        }
        if (flag)
        {
          return (0 * printf("%d %d\n", x, d));
        }
      }
      i += 1;
    }
  }
  return (0 * printf("-1\n"));
}
