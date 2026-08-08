// Translated from solution.cpp.

func read(x: dynamic)
{
  var c = getchar();
  var f = 0;
  x = 0;
  while ((!isdigit(c)))
  {
    f |= (c == cpp_char("-"));
    c = getchar();
  }
  while (isdigit(c))
  {
    x = ((((x << 1)) + ((x << 3))) + ((c ^ 48)));
    c = getchar();
  }
  if (f)
  {
    x = (-x);
  }
  return x;
}

func read(x: dynamic, a: dynamic...)
{
  read(x);
  read(cpp_expand(a));
}

func write(x: dynamic)
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    write((-x));
  } else
  {
    if ((x > 9))
    {
      write((x / 10));
    }
    putchar((cpp_char("0") + (x % 10)));
  }
}

var mod = (1e9 + 7);

var N = (1e6 + 5);

var pre = cpp_array(N);

var f = cpp_array(N);

var fac = cpp_array(N);

var sum: dynamic;

var n: dynamic;

var h: dynamic;

var d: dynamic;

func calc(l: dynamic, r: dynamic)
{
  return if ((l > 0)) ((((pre[r] - pre[(l - 1)]) + mod)) % mod) else pre[r];
}

func main()
{
  read(n, h, d);
  fac[0] = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      fac[i] = (((1 * fac[(i - 1)]) * i) % mod);
      sum = (((sum + fac[i])) % mod);
      i += 1;
    }
  }
  pre[0] = cpp_assign(f[0], "=", fac[n]);
  {
    var i = 1;
    while ((i < h))
    {
      f[i] = (((1 * calc((i - d), (i - 1))) * sum) % mod);
      pre[i] = (((pre[(i - 1)] + f[i])) % mod);
      i += 1;
    }
  }
  write(calc((h - d), (h - 1)));
}
