// Translated from solution.cpp.

var mo: dynamic;

var ans = cpp_array(1000);

var res = cpp_array(1000);

func mul(a: dynamic, b: dynamic)
{
  return if ((b == 0)) 0 else (((((mul(a, (b >> 16)) << 16)) + (a * ((b & (((1 << 16)) - 1)))))) % mo);
}

func mul(f: dynamic, g: dynamic)
{
  var a = cpp_array(2, 2);
  a[0][0] = (((mul(f[0][0], g[0][0]) + mul(f[0][1], g[1][0]))) % mo);
  a[0][1] = (((mul(f[0][0], g[0][1]) + mul(f[0][1], g[1][1]))) % mo);
  a[1][0] = (((mul(f[1][0], g[0][0]) + mul(f[1][1], g[1][0]))) % mo);
  a[1][1] = (((mul(f[1][0], g[0][1]) + mul(f[1][1], g[1][1]))) % mo);
  memcpy(f, a, cpp_sizeof(a));
}

func prezro(g: dynamic)
{
  g[0][0] = cpp_assign(g[1][1], "=", 1);
  g[0][1] = cpp_assign(g[1][0], "=", 0);
}

func quick(f: dynamic, n: dynamic)
{
  var g = cpp_array(2, 2);
  {
    prezro(g);
    while (n)
    {
      if ((n & 1))
      {
        mul(g, f);
      }
      mul(f, f);
      n >>= 1;
    }
  }
  memcpy(f, g, cpp_sizeof(g));
}

func preget(f: dynamic)
{
  f[0][0] = 0;
  f[0][1] = cpp_assign(f[1][0], "=", cpp_assign(f[1][1], "=", 1));
}

func dw(g: dynamic)
{
  return ((((g[0][0] == 1) && (g[1][1] == 1)) && (!g[0][1])) && (!g[1][0]));
}

func main()
{
  var n: dynamic;
  var tm: dynamic;
  var f = cpp_array(2, 2);
  var g = cpp_array(2, 2);
  var h = cpp_array(2, 2);
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var cnt: dynamic;
  var pp: dynamic;
  scanf("%I64d", (&n));
  {
    mo = cpp_assign(tm, "=", 1);
    cnt = 1;
    ans[0] = 0;
    i = 0;
    while ((i < 13))
    {
      mo = (mo * 10);
      preget(f);
      quick(f, tm);
      prezro(g);
      pp = 0;
      j = 0;
      while (true)
      {
        {
          k = 0;
          while ((k < cnt))
          {
            preget(h);
            quick(h, (ans[k] + (tm * j)));
            if ((h[1][0] == (n % mo)))
            {
              res[cpp_update(pp, "++")] = (ans[k] + (tm * j));
            }
            k += 1;
          }
        }
        mul(g, f);
        j += 1;
        if (!(((!dw(g)))))
        {
          break;
        }
      }
      memcpy(ans, res, cpp_sizeof(res));
      cnt = pp;
      tm *= j;
      i += 1;
    }
  }
  printf("%I64d\n", if ((cnt == 0)) -1 else ans[0]);
  return 0;
}
