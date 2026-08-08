// Translated from solution.cpp.

var nmax = 200002;

var n: dynamic;

var k: dynamic;

var a = cpp_array(nmax);

var s = cpp_array(nmax);

var t = cpp_array(nmax);

var c = cpp_array(nmax);

var r = UINT64_MAX;

var A: dynamic;

var B: dynamic;

func main()
{
  scanf("%llu%llu", (&n), (&k));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%llu", (a + i));
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1));
  {
    var i = 1;
    while ((i <= n))
    {
      s[i] = (s[(i - 1)] + a[i]);
      i += 1;
    }
  }
  {
    var i = n;
    while ((i >= 1))
    {
      t[i] = (t[(i + 1)] + a[i]);
      i -= 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((a[i] == a[(i - 1)]))
      {
        c[i] = (c[(i - 1)] + 1);
      } else
      {
        c[i] = 1;
      }
      if ((c[i] >= k))
      {
        puts("0");
        return 0;
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((i >= k))
      {
        A = (((i * a[i]) - s[i]) - ((i - k)));
        r = min(r, A);
      }
      if ((((n - i) + 1) >= k))
      {
        B = ((t[i] - ((((n - i) + 1)) * a[i])) - ((((n - i) + 1) - k)));
        r = min(r, B);
      }
      r = min(r, (((((i * a[i]) - s[i]) + t[i]) - ((((n - i) + 1)) * a[i])) - ((n - k))));
      i += 1;
    }
  }
  printf("%llu\n", r);
  return 0;
}
