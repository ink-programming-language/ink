// Translated from solution.cpp.

func read(x: dynamic) -> dynamic
{
  var c: dynamic = getchar();
  var f: dynamic = 0;
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

func read(x: dynamic, a: dynamic...) -> dynamic
{
  read(x);
  read(cpp_expand(a));
}

func write(x: dynamic) -> dynamic
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

var mod: dynamic = (1e9 + 7);

var N: dynamic = (1e6 + 5);

var pre: dynamic = cpp_array(N);

var f: dynamic = cpp_array(N);

var fac: dynamic = cpp_array(N);

var sum: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var h: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

func calc(l: dynamic, r: dynamic) -> dynamic
{
  return  ((l > 0)) ? ((((pre[r] - pre[(l - 1)]) + mod)) % mod) : pre[r];
}

func main() -> dynamic
{
  read(n, h, d);
  fac[0] = 1;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      fac[i] = (((1 * fac[(i - 1)]) * i) % mod);
      sum = (((sum + fac[i])) % mod);
      i += 1;
    }
  }
  pre[0] = cpp_assign(f[0], "=", fac[n]);
  {
    var i: dynamic = 1;
    while ((i < h))
    {
      f[i] = (((1 * calc((i - d), (i - 1))) * sum) % mod);
      pre[i] = (((pre[(i - 1)] + f[i])) % mod);
      i += 1;
    }
  }
  write(calc((h - d), (h - 1)));
}
