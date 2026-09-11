// Translated from solution.cpp.

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 0;
  var c: dynamic = getchar();
  while (((c < cpp_char("0")) || (c > cpp_char("9"))))
  {
    if ((c == cpp_char("-")))
    {
      f = 1;
    }
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = ((((x << 3)) + ((x << 1))) + ((c ^ cpp_char("0"))));
    c = getchar();
  }
  return  (f) ? (-x) : x;
}

func write(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  if ((x > 9))
  {
    write((x / 10));
  }
  putchar(((x % 10) + cpp_char("0")));
}

var N: dynamic = (2e5 + 10);

var s: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(N);

var maxn: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  n = read();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      s[i] = (s[(i - 1)] + read());
      i += 1;
    }
  }
  maxn = s[n];
  {
    var i: dynamic = (n - 1);
    while ((i >= 1))
    {
      f[i] = maxn;
      maxn = max(maxn, (s[i] - f[i]));
      i -= 1;
    }
  }
  write(f[1]);
  return 0;
}
