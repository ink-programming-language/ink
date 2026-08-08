// Translated from solution.cpp.

var N = (1e5 + 5);

func read(x: dynamic)
{
  x = 0;
  var f = 1;
  var c = getchar();
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

var n: dynamic;

func solve(n: dynamic, k: dynamic)
{
  if ((n == 1))
  {
    return 0;
  }
  return (solve((n - (n / 2)), (k << 1)) + (((n / 2)) * k));
}

func main()
{
  read(n);
  printf("%lld\n", solve(n, 1));
  return 0;
}
