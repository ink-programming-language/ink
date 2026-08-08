// Translated from solution.cpp.

func rd(x: dynamic)
{
  x = 0;
  var f = 1;
  var ch = getchar();
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

func lrd(x: dynamic)
{
  x = 0;
  var f = 1;
  var ch = getchar();
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

var INF = 1e9;

var LINF = 1e18;

var N = 2050;

var n: dynamic;

var m: dynamic;

var l = cpp_array(N);

var s = cpp_array(N);

var c = cpp_array((N << 1));

var f = cpp_array(N, (N << 1));

var ans: dynamic;

var mx = cpp_array(N);

func main()
{
  rd(n);
  rd(m);
  {
    var i = 1;
    while ((i <= n))
    {
      rd(l[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      rd(s[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= (n + m)))
    {
      rd(c[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      s[i] = (c[l[i]] - s[i]);
      i += 1;
    }
  }
  memset(f, -0x7f, cpp_sizeof((f)));
  {
    var i = 1;
    while ((i <= (m + 20)))
    {
      f[i][0] = 0;
      i += 1;
    }
  }
  {
    var i = n;
    while ((i >= 1))
    {
      var u = l[i];
      {
        var j = mx[u];
        while ((j >= 0))
        {
          var x = (f[u][j] + s[i]);
          var y = (j + 1);
          {
            var o = u;
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
        var i = 1;
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
