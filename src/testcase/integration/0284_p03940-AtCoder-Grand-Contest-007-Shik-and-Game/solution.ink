// Translated from solution.cpp.

var ll = dynamic;

var inf = cpp_expression("//Love a");

var N = cpp_expression("//Love");

func read()
{
  var s = 0;
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
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    s = (((s * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (f * s);
}

var f = cpp_array(N);

var mn: dynamic;

var t: dynamic;

var e: dynamic;

var n: dynamic;

var p = cpp_array(N);

func main()
{
  n = read();
  e = read();
  t = read();
  var l = 0;
  mn = 1e18;
  {
    var i = 1;
    while ((i <= n))
    {
      p[i] = read();
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      while (((l <= i) && ((2 * ((p[i] - p[(l + 1)]))) > t)))
      {
        mn = min(mn, (f[l] - (2 * p[(l + 1)])));
        l += 1;
      }
      if ((l < i))
      {
        f[i] = (f[l] + t);
      }
      f[i] = min(f[i], (mn + (2 * p[i])));
      i += 1;
    }
  }
  printf("%lld\n", (f[n] + e));
  return 0;
}
