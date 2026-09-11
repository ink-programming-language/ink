// Translated from solution.cpp.

func f(b: dynamic, n: dynamic) -> dynamic
{
  return  ((b <= n)) ? (f(b, (n / b)) + (n % b)) : n;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, s);
  var res: dynamic = -1;
  var b: dynamic = 2;
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
