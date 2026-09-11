// Translated from solution.cpp.

var kMaxn: dynamic = 100010;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(kMaxn);

var g: dynamic = cpp_array(kMaxn);

var h: dynamic = cpp_array(kMaxn);

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var now: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (a + i));
      i += 1;
    }
  }
  m = 0;
  memset(g, 0, cpp_sizeof((g)));
  memset(h, 0, cpp_sizeof((h)));
  {
    i = 1;
    while ((i <= n))
    {
      if ((!g[i]))
      {
        if (g[a[i]])
        {
          g[i] = g[a[i]];
        } else
        {
          g[i] = cpp_assign(g[a[i]], "=", cpp_update(m, "++"));
          h[m] = a[i];
        }
      }
      i += 1;
    }
  }
  ans = 0;
  {
    i = 1;
    while ((i <= n))
    {
      if ((h[g[i]] != a[i]))
      {
        ans = -1;
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= m))
    {
      if ((g[h[i]] != i))
      {
        ans = -1;
      }
      i += 1;
    }
  }
  if ((ans == -1))
  {
    puts("-1");
    return 0;
  }
  printf("%d\n", m);
  {
    i = 1;
    while ((i <= n))
    {
      printf("%d ", g[i]);
      i += 1;
    }
  }
  puts("");
  {
    i = 1;
    while ((i <= m))
    {
      printf("%d ", h[i]);
      i += 1;
    }
  }
  return 0;
}
