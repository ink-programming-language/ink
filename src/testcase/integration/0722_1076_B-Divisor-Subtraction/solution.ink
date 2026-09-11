// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var ans: dynamic = 0;
  if (((n % 2) == 0))
  {
    write((n / 2), "\n");
    return 0;
  }
  {
    var i: dynamic = 2;
    while (((i * i) <= n))
    {
      if (((n % i) == 0))
      {
        write(((((n - i)) / 2) + 1), "\n");
        return 0;
      }
      i += 1;
    }
  }
  write(1, "\n");
}
