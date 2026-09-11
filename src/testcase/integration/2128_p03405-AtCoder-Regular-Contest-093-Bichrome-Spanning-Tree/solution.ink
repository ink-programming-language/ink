// Translated from solution.cpp.

var ll: dynamic = dynamic;

func read() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
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

var MN: dynamic = cpp_expression("#inc");

var MM: dynamic = cpp_expression("#inc");

var mod: dynamic = cpp_expression("#include<c");

class edge
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array((MM + 5));

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return (a.w < b.w);
}

var f: dynamic = cpp_array((MN + 5));

func gf(k: dynamic) -> dynamic
{
  return  (f[k]) ? cpp_assign(f[k], "=", gf(f[k])) : k;
}

var ans: dynamic = 1;

func pow(x: dynamic, k: dynamic) -> dynamic
{
  var sum: dynamic = 1;
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

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var s1: dynamic = 0;
  var s2: dynamic = 1;
  var s: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
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
