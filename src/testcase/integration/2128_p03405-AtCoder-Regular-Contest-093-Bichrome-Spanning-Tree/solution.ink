// Translated from solution.cpp.

var ll = dynamic;

func read()
{
  var x: dynamic;
  var c: dynamic;
  while ((((cpp_assign(c, "=", getchar())) < cpp_char("0")) || (c > cpp_char("9"))))
  {
  }
  {
    x = (c - cpp_char("0"));
    while ((((cpp_assign(c, "=", getchar())) >= cpp_char("0")) && (c <= cpp_char("9"))))
    {
      x = (((x * 10) + c) - cpp_char("0"));
    }
  }
  return x;
}

var MN = cpp_expression("#inc");

var MM = cpp_expression("#inc");

var mod = cpp_expression("#include<c");

class edge
{
  var x: dynamic;
  var y: dynamic;
  var w: dynamic;
}

var e = cpp_array((MM + 5));

func cmp(a: dynamic, b: dynamic)
{
  return (a.w < b.w);
}

var f = cpp_array((MN + 5));

func gf(k: dynamic)
{
  return if (f[k]) cpp_assign(f[k], "=", gf(f[k])) else k;
}

var ans = 1;

func pow(x: dynamic, k: dynamic)
{
  var sum = 1;
  {
    while (k)
    {
      if ((k & 1))
      {
        sum = (((1 * sum) * x) % mod);
      }
      k >>= 1;
      x = (((1 * x) * x) % mod);
    }
  }
  return sum;
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var s1 = 0;
  var s2 = 1;
  var s: dynamic;
  var x: dynamic;
  n = read();
  m = read();
  x = read();
  {
    i = 1;
    while ((i <= m))
    {
      e[i].x = read();
      e[i].y = read();
      e[i].w = read();
      i += 1;
    }
  }
  sort((e + 1), ((e + m) + 1), cmp);
  {
    i = 2;
    while ((i <= m))
    {
      memset(f, cpp_assign(s, "=", 0), cpp_sizeof((f)));
      if ((gf(e[i].x) != gf(e[i].y)))
      {
        f[gf(e[i].x)] = gf(e[i].y);
        s += e[i].w;
      }
      {
        j = 1;
        while ((j <= m))
        {
          if ((gf(e[j].x) != gf(e[j].y)))
          {
            f[gf(e[j].x)] = gf(e[j].y);
            s += e[j].w;
          }
          j += 1;
        }
      }
      if ((s == x))
      {
        s1 += 1;
      }
      if ((s > x))
      {
        ans = (((2 * ans)) % mod);
      }
      i += 1;
    }
  }
  printf("%d", (((2 * ans) * ((pow(2, s1) - 1))) % mod));
}
