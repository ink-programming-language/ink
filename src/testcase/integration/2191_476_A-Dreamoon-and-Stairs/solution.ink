// Translated from solution.cpp.

func main()
{
  var m: dynamic;
  var n: dynamic;
  var i: dynamic;
  var x: dynamic;
  read(n, m);
  if ((n < m))
  {
    write(-1);
    return 0;
  }
  if ((n == m))
  {
    write(n);
    return 0;
  }
  if (((n % 2) == 0))
  {
    x = (n / 2);
    if (((x % m) == 0))
    {
      write(x);
      return 0;
    }
    {
      i = (x + 1);
      while ((i <= n))
      {
        if (((i % m) == 0))
        {
          write(i);
          return 0;
        }
        i += 1;
      }
    }
  } else
  {
    x = (((n - 1)) / 2);
    x = (x + 1);
    if (((x % m) == 0))
    {
      write(x);
      return 0;
    }
    {
      i = (x + 1);
      while ((i <= n))
      {
        if (((i % m) == 0))
        {
          write(i);
          return 0;
        }
        i += 1;
      }
    }
  }
  write(-1);
}
