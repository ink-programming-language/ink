// Translated from solution.cpp.

var MAXN = (1e6 + 10);

var a = cpp_array(MAXN);

var d = cpp_array(MAXN);

var cnt = cpp_array(MAXN);

var ans: dynamic;

var n: dynamic;

var siz: dynamic;

func cut(x: dynamic)
{
  siz = 0;
  {
    var i = 1;
    while (((i * i) <= x))
    {
      if (((x % i) == 0))
      {
        d[cpp_update(siz, "++")] = i;
        if ((x != (i * i)))
        {
          d[cpp_update(siz, "++")] = (x / i);
        }
      }
      i += 1;
    }
  }
  memset(cnt, 0, cpp_sizeof((cnt)));
}

func gcd(a: dynamic, b: dynamic)
{
  return if (b) gcd(b, (a % b)) else a;
}

func random(x: dynamic, y: dynamic)
{
  return (((cpp_cast(rand()) * rand()) % (((y - x) + 1))) + x);
}

func main()
{
  srand(time(null));
  scanf("%I64d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%I64d", (&a[i]));
      i += 1;
    }
  }
  {
    var T = 1;
    while ((T <= 10))
    {
      var x = a[random(1, n)];
      cut(x);
      sort((d + 1), ((d + siz) + 1));
      {
        var i = 1;
        while ((i <= n))
        {
          var pos = (lower_bound((d + 1), ((d + siz) + 1), gcd(x, a[i])) - d);
          cnt[pos] += 1;
          i += 1;
        }
      }
      {
        var i = 1;
        while ((i <= siz))
        {
          {
            var j = (i + 1);
            while ((j <= siz))
            {
              if (((d[j] % d[i]) == 0))
              {
                cnt[i] += cnt[j];
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      {
        var i = siz;
        while ((i >= 1))
        {
          if (((cnt[i] * 2) >= n))
          {
            ans = max(ans, d[i]);
            break;
          }
          i -= 1;
        }
      }
      T += 1;
    }
  }
  printf("%I64d\n", ans);
  return 0;
}
