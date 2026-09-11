// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

func pie(x: dynamic, y: dynamic) -> dynamic
{
  while ((x != y))
  {
    if ((x > y))
    {
      x = (x - y);
    }
    if ((x < y))
    {
      y = (y - x);
    }
  }
  if ((x == 1))
  {
    return 1;
  } else
  {
    return 0;
  }
}

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = (n / 2);
    while ((i >= 1))
    {
      if ((pie(i, (n - i)) == 1))
      {
        write(i, " ", (n - i));
        return 0;
      }
      i -= 1;
    }
  }
  return 0;
}
