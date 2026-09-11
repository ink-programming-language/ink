// Translated from solution.cpp.

func rd(x: dynamic) -> dynamic
{
  x = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (((ch <= cpp_char("9")) && (ch >= cpp_char("0"))))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  x *= f;
}

func lrd(x: dynamic) -> dynamic
{
  x = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (((ch <= cpp_char("9")) && (ch >= cpp_char("0"))))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  x *= f;
}

var INF: dynamic = 1e9;

var LINF: dynamic = 1e18;

var N: dynamic = 2050;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var l: dynamic = cpp_array(N);

var s: dynamic = cpp_array(N);

var c: dynamic = cpp_array((N << 1));

var f: dynamic = cpp_array(N, (N << 1));

var ans: dynamic = cpp_uninitialized();

var mx: dynamic = cpp_array(N);

func main() -> dynamic
{
  rd(n);
  rd(m);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      rd(l[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      rd(s[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= (n + m)))
    {
      rd(c[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      s[i] = (c[l[i]] - s[i]);
      i += 1;
    }
  }
  memset(f, -0x7f, cpp_sizeof((f)));
  {
    var i: dynamic = 1;
    while ((i <= (m + 20)))
    {
      f[i][0] = 0;
      i += 1;
    }
  }
  {
    var i: dynamic = n;
    while ((i >= 1))
    {
      var u: dynamic = l[i];
      {
        var j: dynamic = mx[u];
        while ((j >= 0))
        {
          var x: dynamic = (f[u][j] + s[i]);
          var y: dynamic = (j + 1);
          {
            var o: dynamic = u;
            while (y)
            {
              mx[o] = max(mx[o], y);
              f[o][y] = max(f[o][y], x);
              o += 1;
              y /= 2;
              x += (c[o] * y);
            }
          }
          ans = max(ans, x);
          j -= 1;
        }
      }
      {
        var i: dynamic = 1;
        while ((i <= (m + 20)))
        {
          f[i][0] = max(f[i][0], max(f[(i - 1)][0], f[(i - 1)][1]));
          i += 1;
        }
      }
      i -= 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
