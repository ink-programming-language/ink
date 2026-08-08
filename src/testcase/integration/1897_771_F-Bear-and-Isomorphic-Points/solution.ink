// Translated from solution.cpp.

var N = (5e5 + 7);

var eps = 1e-7;

var t: dynamic;

var n: dynamic;

var q: dynamic;

var k: dynamic;

var s: dynamic;

var op: dynamic;

var x = cpp_array(N);

var y = cpp_array(N);

var xa = cpp_array(N);

var ya = cpp_array(N);

class node
{
  var x: dynamic;
  var y: dynamic;
}

var e = cpp_array(N);

var f = cpp_array(N);

class vec
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
}

var w = cpp_array(N);

var g = cpp_array(N);

var h = cpp_array(N);

class vat
{
  var x: dynamic;
  var y: dynamic;
}

var gt = cpp_array(N);

func read()
{
  var num = 0;
  var t = cpp_char("+");
  var g = getchar();
  while (((g < 48) || (57 < g)))
  {
    t = g;
    g = getchar();
  }
  while (((47 < g) && (g < 58)))
  {
    num = (((num * 10) + g) - 48);
    g = getchar();
  }
  if ((t == cpp_char("-")))
  {
    return (-num);
  }
  return num;
}

func cmp(a: dynamic, b: dynamic)
{
  return (atan2(a.y, a.x) < atan2(b.y, b.x));
}

func dmp(a: dynamic, b: dynamic)
{
  return (atan2(a.y, a.x) > atan2(b.y, b.x));
}

func tmp(a: dynamic, b: dynamic)
{
  return (atan2((a.d - a.b), (a.c - a.a)) < atan2((b.d - b.b), (b.c - b.a)));
}

func qmp(a: dynamic, b: dynamic)
{
  if ((a.x == b.x))
  {
    return (a.y < b.y);
  }
  return (a.x < b.x);
}

func getvac(a: dynamic, b: dynamic)
{
  return ((a.x * b.y) - (a.y * b.x));
}

func adds(a: dynamic, b: dynamic)
{
  swap(a, b);
  q += 1;
  w[q].a = (a.x + x[1]);
  w[q].b = (a.y + y[1]);
  w[q].c = (b.x + x[1]);
  w[q].d = (b.y + y[1]);
}

func eps_check(u: dynamic)
{
  if ((((-eps) <= u) && (u <= eps)))
  {
    return 1;
  }
  return 0;
}

func node_check(u: dynamic, x: dynamic, y: dynamic)
{
  var a: dynamic;
  var b: dynamic;
  a.x = (u.c - u.a);
  a.y = (u.d - u.b);
  b.x = (x - u.a);
  b.y = (y - u.b);
  return ((getvac(a, b) >= (-eps)));
}

func getnode(a: dynamic, b: dynamic)
{
  var z: dynamic;
  var f1 = (((a.d - a.b)) * ((b.c - b.a)));
  var f2 = (((b.d - b.b)) * ((a.c - a.a)));
  var f3 = ((((a.b - b.b)) * ((b.c - b.a))) * ((a.c - a.a)));
  f3 = ((f3 - (f1 * a.a)) + (f2 * b.a));
  z.x = (f3 / ((f2 - f1)));
  if (eps_check((a.a - a.c)))
  {
    swap(a, b);
  }
  z.y = (((((z.x - a.a)) * ((a.d - a.b))) / ((a.c - a.a))) + a.b);
  return z;
}

func check(a: dynamic, b: dynamic, c: dynamic)
{
  var z = getnode(b, c);
  return node_check(a, z.x, z.y);
}

func getvat(a: dynamic, b: dynamic)
{
  a.x -= b.x;
  a.y -= b.y;
  b.x = (-b.x);
  b.y = (-b.y);
  return ((getvac(a, b) <= eps));
}

func getvas(a: dynamic, b: dynamic)
{
  a.x -= b.x;
  a.y -= b.y;
  b.x = (-b.x);
  b.y = (-b.y);
  return ((getvac(a, b) <= (-eps)));
}

func gcd(a: dynamic, b: dynamic)
{
  if ((!b))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func main()
{
  t = read();
  var og = 0;
  while (cpp_update(t, "--"))
  {
    n = read();
    q = 0;
    k = 0;
    op = 0;
    var fl = 0;
    {
      var i = 1;
      while ((i <= n))
      {
        xa[i] = cpp_assign(x[i], "=", read());
        ya[i] = cpp_assign(y[i], "=", read());
        i += 1;
      }
    }
    {
      var i = 2;
      while ((i <= n))
      {
        x[i] = (x[i] - x[1]);
        y[i] = (y[i] - y[1]);
        e[(i - 1)].x = x[i];
        e[(i - 1)].y = y[i];
        i += 1;
      }
    }
    n -= 1;
    {
      var i = 2;
      while ((i <= (n + 1)))
      {
        var z = gcd(abs(x[i]), abs(y[i]));
        x[i] /= z;
        y[i] /= z;
        if ((x[i] < 0))
        {
          x[i] = (-x[i]);
          y[i] = (-y[i]);
        }
        if ((x[i] == 0))
        {
          y[i] = 1;
        }
        if ((y[i] == 0))
        {
          x[i] = 1;
        }
        op += 1;
        gt[op].x = x[i];
        gt[op].y = y[i];
        i += 1;
      }
    }
    sort((gt + 1), ((gt + op) + 1), qmp);
    {
      var i = 2;
      while ((i <= op))
      {
        if (((gt[(i - 1)].x == gt[i].x) && (gt[(i - 1)].y == gt[i].y)))
        {
          fl = 1;
        }
        i += 1;
      }
    }
    sort((e + 1), ((e + n) + 1), cmp);
    {
      var i = 1;
      while ((i <= n))
      {
        e[(i + n)] = e[i];
        i += 1;
      }
    }
    var j = 1;
    {
      var i = 1;
      while ((i <= n))
      {
        j = max(j, (i + 1));
        while (((j <= ((i + n) - 2)) && getvat(e[i], e[(j + 1)])))
        {
          j += 1;
        }
        if (getvat(e[i], e[j]))
        {
          adds(e[i], e[j]);
        }
        if (getvat(e[i], e[(i + 1)]))
        {
          adds(e[i], e[(i + 1)]);
        }
        i += 1;
      }
    }
    q += 1;
    w[q].a = -1000000;
    w[q].b = 1000000;
    w[q].c = 1000000;
    w[q].d = 1000000;
    q += 1;
    w[q].a = 1000000;
    w[q].b = 1000000;
    w[q].c = 1000000;
    w[q].d = -1000000;
    q += 1;
    w[q].a = 1000000;
    w[q].b = -1000000;
    w[q].c = -1000000;
    w[q].d = -1000000;
    q += 1;
    w[q].a = -1000000;
    w[q].b = -1000000;
    w[q].c = -1000000;
    w[q].d = 1000000;
    sort((w + 1), ((w + q) + 1), tmp);
    g[1] = w[1];
    k = 1;
    {
      var i = 2;
      while ((i <= q))
      {
        if (eps_check((atan2((w[i].d - w[i].b), (w[i].c - w[i].a)) - atan2((g[k].d - g[k].b), (g[k].c - g[k].a)))))
        {
          if (node_check(w[i], g[k].a, g[k].b))
          {
            g[k] = w[i];
          }
        } else
        {
          g[cpp_update(k, "++")] = w[i];
        }
        i += 1;
      }
    }
    var l = 1;
    var r = 2;
    {
      var i = 1;
      while ((i <= 2))
      {
        h[i] = g[i];
        i += 1;
      }
    }
    {
      var i = 3;
      while ((i <= k))
      {
        if ((i == 3))
        {
          var a: dynamic;
          var b: dynamic;
          a = getnode(h[1], h[2]);
          b = getnode(h[2], h[1]);
        }
        while ((((r - l) >= 1) && check(g[i], h[r], h[(r - 1)])))
        {
          r -= 1;
        }
        while ((((r - l) >= 1) && check(g[i], h[l], h[(l + 1)])))
        {
          l += 1;
        }
        h[cpp_update(r, "++")] = g[i];
        i += 1;
      }
    }
    while ((((r - l) >= 2) && check(h[l], h[r], h[(r - 1)])))
    {
      r -= 1;
    }
    if ((((r - l) <= 1) || fl))
    {
      if ((!og))
      {
        printf("0\n");
      }
    } else
    {
      s = 0;
      var ans = 0;
      {
        var i = l;
        while ((i < r))
        {
          f[cpp_update(s, "++")] = getnode(h[i], h[(i + 1)]);
          i += 1;
        }
      }
      f[cpp_update(s, "++")] = getnode(h[l], h[r]);
      {
        var i = 2;
        while ((i <= s))
        {
          f[i].y -= f[1].y;
          f[i].x -= f[1].x;
          i += 1;
        }
      }
      {
        var i = 2;
        while ((i < s))
        {
          ans = (ans + (abs(getvac(f[i], f[(i + 1)])) / 2));
          i += 1;
        }
      }
      printf("%.9Lf\n", ans);
    }
  }
}
