// Translated from solution.cpp.

func f(b: dynamic, n: dynamic)
{
  return if ((b <= n)) (f(b, (n / b)) + (n % b)) else n;
}

func main()
{
  var n: dynamic;
  var s: dynamic;
  read(n, s);
  var res = -1;
  var b = 2;
  {
    while (((((b - 1)) * ((b - 1))) <= n))
    {
      if ((f(b, n) == s))
      {
        res = b;
        cpp_goto("goto out;");
      }
      b += 1;
    }
  }
  {
    while ((b > 0))
    {
      if (((((((((n - s) + b)) % b) == 0) && (b < ((((n - s) + b)) / b))) && (0 <= (s - b))) && ((s - b) < ((((n - s) + b)) / b))))
      {
        res = ((((n - s) + b)) / b);
        cpp_goto("goto out;");
      }
      b -= 1;
    }
  }
  if ((n == s))
  {
    res = (n + 1);
  }
  write(res, "\n");
}
