// Translated from solution.cpp.

var mo: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_array(1000);

var res: dynamic = cpp_array(1000);

func mul(a: dynamic, b: dynamic) -> dynamic
{
  return  ((b == 0)) ? 0 : (((((mul(a, (b >> 16)) << 16)) + (a * ((b & (((1 << 16)) - 1)))))) % mo);
}

func mul(f: dynamic, g: dynamic) -> dynamic
{
  var a: dynamic = cpp_array(2, 2);
  a[0][0] = (((mul(f[0][0], g[0][0]) + mul(f[0][1], g[1][0]))) % mo);
  a[0][1] = (((mul(f[0][0], g[0][1]) + mul(f[0][1], g[1][1]))) % mo);
  a[1][0] = (((mul(f[1][0], g[0][0]) + mul(f[1][1], g[1][0]))) % mo);
  a[1][1] = (((mul(f[1][0], g[0][1]) + mul(f[1][1], g[1][1]))) % mo);
  memcpy(f, a, cpp_sizeof(a));
}

func prezro(g: dynamic) -> dynamic
{
  g[0][0] = cpp_assign(g[1][1], "=", 1);
  g[0][1] = cpp_assign(g[1][0], "=", 0);
}

func quick(f: dynamic, n: dynamic) -> dynamic
{
  var g: dynamic = cpp_array(2, 2);
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

func preget(f: dynamic) -> dynamic
{
  f[0][0] = 0;
  f[0][1] = cpp_assign(f[1][0], "=", cpp_assign(f[1][1], "=", 1));
}

func dw(g: dynamic) -> dynamic
{
  return ((((g[0][0] == 1) && (g[1][1] == 1)) && (!g[0][1])) && (!g[1][0]));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var tm: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_array(2, 2);
  var g: dynamic = cpp_array(2, 2);
  var h: dynamic = cpp_array(2, 2);
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var cnt: dynamic = cpp_uninitialized();
  var pp: dynamic = cpp_uninitialized();
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
  printf("%I64d\n",  ((cnt == 0)) ? -1 : ans[0]);
  return 0;
}
