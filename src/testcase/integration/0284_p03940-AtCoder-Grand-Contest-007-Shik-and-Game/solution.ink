// Translated from solution.cpp.

var ll: dynamic = dynamic;

var inf: dynamic = cpp_expression("//Love a");

var N: dynamic = cpp_expression("//Love");

func read() -> dynamic
{
  var s: dynamic = 0;
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
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    s = (((s * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (f * s);
}

var f: dynamic = cpp_array(N);

var mn: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var e: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(N);

func main() -> dynamic
{
  n = read();
  e = read();
  t = read();
  var l: dynamic = 0;
  mn = 1e18;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      p[i] = read();
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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
