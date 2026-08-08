// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var ans = 0;
  if (((n % 2) == 0))
  {
    write((n / 2), "\n");
    return 0;
  }
  {
    var i = 2;
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
