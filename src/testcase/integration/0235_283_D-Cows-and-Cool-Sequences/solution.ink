// Translated from solution.cpp.

var N: dynamic = 5005;

var INF: dynamic = 0x3f3f3f3f;

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var g: dynamic = cpp_array(N, 2);

var dp: dynamic = cpp_array(N, 2);

func read() -> dynamic
{
  var f: dynamic = 1;
  var x: dynamic = 0;
  var ch: dynamic = getchar();
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

func upd(x: dynamic, y: dynamic) -> dynamic
{
  x = min(x, y);
}

func f(x: dynamic) -> dynamic
{
  if (((x % 2) == 0))
  {
    return (x / 2);
  }
  return x;
}

func main() -> dynamic
{
  n = read();
  a[0] = 1;
  {
    var i: dynamic = (n);
    var iend: dynamic = (1);
    while ((i >= iend))
    {
      a[i] = read();
      i -= 1;
    }
  }
  dp[0][0] = 0;
  g[0][0] = 1;
  {
    var i: dynamic = (1);
    var iend: dynamic = (n);
    while ((i <= iend))
    {
      {
        var j: dynamic = (0);
        var jend: dynamic = (i);
        while ((j <= jend))
        {
          dp[(i & 1)][j] = INF;
          j += 1;
        }
      }
      {
        var j: dynamic = (0);
        var jend: dynamic = ((i - 1));
        while ((j <= jend))
        {
          var t: dynamic = (i & 1);
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
  var ans: dynamic = INF;
  {
    var i: dynamic = (0);
    var iend: dynamic = (n);
    while ((i <= iend))
    {
      upd(ans, dp[(n & 1)][i]);
      i += 1;
    }
  }
  printf("%d", ans);
  return 0;
}
