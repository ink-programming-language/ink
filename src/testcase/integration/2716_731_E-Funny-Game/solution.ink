// Translated from solution.cpp.

func read()
{
  var x = 0;
  var f = 0;
  var c = getchar();
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
  return if (f) (-x) else x;
}

func write(x: dynamic)
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

var N = (2e5 + 10);

var s = cpp_array(N);

var n: dynamic;

var f = cpp_array(N);

var maxn: dynamic;

func main()
{
  n = read();
  {
    var i = 1;
    while ((i <= n))
    {
      s[i] = (s[(i - 1)] + read());
      i += 1;
    }
  }
  maxn = s[n];
  {
    var i = (n - 1);
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
