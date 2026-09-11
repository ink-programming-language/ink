// Translated from solution.cpp.

var N: dynamic = (1e5 + 5);

func read(x: dynamic) -> dynamic
{
  x = 0;
  var f: dynamic = 1;
  var c: dynamic = getchar();
  {
    while ((!isdigit(c)))
    {
      if ((c == cpp_char("-")))
      {
        f = -1;
      }
      c = getchar();
    }
  }
  {
    while (isdigit(c))
    {
      x = (((((x << 1)) + ((x << 3))) + c) - cpp_char("0"));
      c = getchar();
    }
  }
  x *= f;
}

var n: dynamic = cpp_uninitialized();

func solve(n: dynamic, k: dynamic) -> dynamic
{
  if ((n == 1))
  {
    return 0;
  }
  return (solve((n - (n / 2)), (k << 1)) + (((n / 2)) * k));
}

func main() -> dynamic
{
  read(n);
  printf("%lld\n", solve(n, 1));
  return 0;
}
