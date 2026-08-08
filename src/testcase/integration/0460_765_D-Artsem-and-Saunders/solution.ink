// Translated from solution.cpp.

var kMaxn = 100010;

var n: dynamic;

var m: dynamic;

var a = cpp_array(kMaxn);

var g = cpp_array(kMaxn);

var h = cpp_array(kMaxn);

func main()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var x: dynamic;
  var now: dynamic;
  var ans: dynamic;
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
