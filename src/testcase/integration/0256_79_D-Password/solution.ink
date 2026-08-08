// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

var m: dynamic;

var x: dynamic;

var s: dynamic;

var p = cpp_array(10005);

var c = cpp_array(105);

var y = cpp_array(30, 30);

var d = cpp_array(10005);

var q = cpp_array(10005);

var f: dynamic;

var r: dynamic;

var g = cpp_array(1100005);

var a = cpp_array(10005);

func work(x: dynamic, y: dynamic)
{
  if ((((x > 0) && (x <= n)) && (y < d[x])))
  {
    d[x] = y;
    q[cpp_update(r, "++")] = x;
  }
}

func bfs(x: dynamic)
{
  var i: dynamic;
  memset(d, 1, cpp_sizeof((d)));
  d[x] = 0;
  f = 0;
  r = 0;
  {
    q[0] = x;
    while ((f <= r))
    {
      {
        i = 1;
        while ((i <= m))
        {
          work((q[f] + c[i]), (d[q[f]] + 1));
          work((q[f] - c[i]), (d[q[f]] + 1));
          i += 1;
        }
      }
      f += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      if (a[i])
      {
        y[p[x]][p[i]] = d[i];
      }
      i += 1;
    }
  }
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var l: dynamic;
  scanf("%d%d%d", (&n), (&k), (&m));
  n += 1;
  {
    i = 1;
    while ((i <= k))
    {
      scanf("%d", (&x));
      a[x] = 1;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= m))
    {
      scanf("%d", (&c[i]));
      i += 1;
    }
  }
  {
    i = n;
    while ((i >= 1))
    {
      if (cpp_assign(a[i], "^=", a[(i - 1)]))
      {
        p[i] = cpp_update(s, "++");
      }
      i -= 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      if (a[i])
      {
        bfs(i);
      }
      i += 1;
    }
  }
  memset(g, 80, cpp_sizeof((g)));
  g[0] = 0;
  {
    i = 1;
    while ((i <= (((1 << s)) - 1)))
    {
      {
        j = 1;
        while ((j <= s))
        {
          if ((i & ((1 << (j - 1)))))
          {
            {
              l = (j + 1);
              while ((l <= s))
              {
                if ((i & ((1 << (l - 1)))))
                {
                  g[i] = min(g[i], (g[((i ^ ((1 << (j - 1)))) ^ ((1 << (l - 1))))] + y[j][l]));
                }
                l += 1;
              }
            }
            break;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((g[(((1 << s)) - 1)] > (1 << 20)))
  {
    puts("-1");
  } else
  {
    printf("%d\n", g[(((1 << s)) - 1)]);
  }
  return 0;
}
