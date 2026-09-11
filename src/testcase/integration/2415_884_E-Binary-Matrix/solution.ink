// Translated from solution.cpp.

func readi(x: dynamic) -> dynamic
{
  var v: dynamic = 0;
  var f: dynamic = 1;
  var c: dynamic = getchar();
  while (((!isdigit(c)) && (c != cpp_char("-"))))
  {
    c = getchar();
  }
  if ((c == cpp_char("-")))
  {
    f = -1;
  } else
  {
    v = (((v * 10) + c) - cpp_char("0"));
  }
  while (isdigit(cpp_assign(c, "=", getchar())))
  {
    v = (((v * 10) + c) - cpp_char("0"));
  }
  x = (v * f);
}

func readll(x: dynamic) -> dynamic
{
  var v: dynamic = 0;
  var f: dynamic = 1;
  var c: dynamic = getchar();
  while (((!isdigit(c)) && (c != cpp_char("-"))))
  {
    c = getchar();
  }
  if ((c == cpp_char("-")))
  {
    f = -1;
  } else
  {
    v = (((v * 10) + c) - cpp_char("0"));
  }
  while (isdigit(cpp_assign(c, "=", getchar())))
  {
    v = (((v * 10) + c) - cpp_char("0"));
  }
  x = (v * f);
}

func readc(x: dynamic) -> dynamic
{
  var c: dynamic = cpp_uninitialized();
  while (((cpp_assign(c, "=", getchar())) == cpp_char(" ")))
  {
  }
  x = c;
}

func writes(s: dynamic) -> dynamic
{
  puts(s.c_str());
}

func writeln() -> dynamic
{
  writes("");
}

func writei(x: dynamic) -> dynamic
{
  if ((!x))
  {
    putchar(cpp_char("0"));
  }
  var a: dynamic = cpp_array(25);
  var top: dynamic = 0;
  while (x)
  {
    a[cpp_update(top, "++")] = (((x % 10)) + cpp_char("0"));
    x /= 10;
  }
  while (top)
  {
    putchar(a[top]);
    top -= 1;
  }
}

func writell(x: dynamic) -> dynamic
{
  if ((!x))
  {
    putchar(cpp_char("0"));
  }
  var a: dynamic = cpp_array(25);
  var top: dynamic = 0;
  while (x)
  {
    a[cpp_update(top, "++")] = (((x % 10)) + cpp_char("0"));
    x /= 10;
  }
  while (top)
  {
    putchar(a[top]);
    top -= 1;
  }
}

var lst: dynamic = cpp_array((((1 << 15)) + 7));

var cur: dynamic = cpp_array((((1 << 15)) + 7));

var fa: dynamic = cpp_array((((1 << 15)) + 7));

var n: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

func find(x: dynamic) -> dynamic
{
  if ((x == fa[x]))
  {
    return x;
  }
  return cpp_assign(fa[x], "=", find(fa[x]));
}

func merge(x: dynamic, y: dynamic) -> dynamic
{
  if ((find(x) == find(y)))
  {
    return 0;
  }
  fa[find(x)] = find(y);
  return 1;
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  read(n, m);
  {
    i = 1;
    while ((i <= n))
    {
      read(s);
      s = (" " + s);
      {
        j = 1;
        while ((j <= (m / 4)))
        {
          if ((s[j] >= cpp_char("A")))
          {
            s[j] -= ((cpp_char("A") - cpp_char("9")) - 1);
          }
          j += 1;
        }
      }
      {
        j = 1;
        while ((j <= m))
        {
          var x: dynamic = (s[((((j - 1)) / 4) + 1)] - cpp_char("0"));
          cur[j] = cpp_cast(((x & ((1 << ((3 - (((j - 1)) % 4))))))));
          ans += cur[j];
          j += 1;
        }
      }
      {
        j = (m + 1);
        while ((j <= (m * 2)))
        {
          fa[j] = j;
          j += 1;
        }
      }
      {
        j = 1;
        while ((j <= m))
        {
          if ((lst[j] && cur[j]))
          {
            ans -= merge(j, (j + m));
          }
          j += 1;
        }
      }
      {
        j = 2;
        while ((j <= m))
        {
          if ((cur[(j - 1)] && cur[j]))
          {
            ans -= merge(((j + m) - 1), (j + m));
          }
          j += 1;
        }
      }
      {
        j = 1;
        while ((j <= m))
        {
          lst[j] = cur[j];
          fa[j] = (find((j + m)) - m);
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans);
  return 0;
}
