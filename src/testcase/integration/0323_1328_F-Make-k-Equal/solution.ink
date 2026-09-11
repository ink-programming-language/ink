// Translated from solution.cpp.

var nmax: dynamic = 200002;

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(nmax);

var s: dynamic = cpp_array(nmax);

var t: dynamic = cpp_array(nmax);

var c: dynamic = cpp_array(nmax);

var r: dynamic = UINT64_MAX;

var A: dynamic = cpp_uninitialized();

var B: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%llu%llu", (&n), (&k));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%llu", (a + i));
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      s[i] = (s[(i - 1)] + a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = n;
    while ((i >= 1))
    {
      t[i] = (t[(i + 1)] + a[i]);
      i -= 1;
    }
  }
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
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
