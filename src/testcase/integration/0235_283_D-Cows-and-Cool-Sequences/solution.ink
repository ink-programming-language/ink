// Translated from solution.cpp.

var N = 5005;

var INF = 0x3f3f3f3f;

var n: dynamic;

var a = cpp_array(N);

var g = cpp_array(N, 2);

var dp = cpp_array(N, 2);

func read()
{
  var f = 1;
  var x = 0;
  var ch = getchar();
  while (((ch > cpp_char("9")) || (ch < cpp_char("0"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (f * x);
}

func upd(x: dynamic, y: dynamic)
{
  x = min(x, y);
}

func f(x: dynamic)
{
  if (((x % 2) == 0))
  {
    return (x / 2);
  }
  return x;
}

func main()
{
  n = read();
  a[0] = 1;
  {
    var i = (n);
    var iend = (1);
    while ((i >= iend))
    {
      a[i] = read();
      i -= 1;
    }
  }
  dp[0][0] = 0;
  g[0][0] = 1;
  {
    var i = (1);
    var iend = (n);
    while ((i <= iend))
    {
      {
        var j = (0);
        var jend = (i);
        while ((j <= jend))
        {
          dp[(i & 1)][j] = INF;
          j += 1;
        }
      }
      {
        var j = (0);
        var jend = ((i - 1));
        while ((j <= jend))
        {
          var t = (i & 1);
          if ((((((2 * a[i])) % g[(t ^ 1)][j]) == 0) && (((((2 * a[i]) / g[(t ^ 1)][j])) % 2) != (g[(t ^ 1)][j] % 2))))
          {
            upd(dp[t][i], dp[(t ^ 1)][j]);
            g[t][i] = a[i];
          }
          upd(dp[t][j], (dp[(t ^ 1)][j] + 1));
          g[t][j] = f(g[(t ^ 1)][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = INF;
  {
    var i = (0);
    var iend = (n);
    while ((i <= iend))
    {
      upd(ans, dp[(n & 1)][i]);
      i += 1;
    }
  }
  printf("%d", ans);
  return 0;
}
