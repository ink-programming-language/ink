// Translated from solution.cpp.

func prime(n: dynamic) -> dynamic
{
  {
    var i: dynamic = 2;
    while ((i < n))
    {
      if (((n % i) == 0))
      {
        return 0;
      }
      i += 1;
    }
  }
  return 1;
}

func sgn(n: dynamic) -> dynamic
{
  if ((n > 0))
  {
    return 1;
  } else
  {
    return -1;
  }
}

func max_digit(n: dynamic) -> dynamic
{
  var lar: dynamic = 0;
  while (n)
  {
    var r: dynamic = (n % 10);
    lar = max(r, lar);
    n = (n / 10);
  }
  return lar;
}

func min_digit(n: dynamic) -> dynamic
{
  var lar: dynamic = 9;
  while (n)
  {
    var r: dynamic = (n % 10);
    lar = min(r, lar);
    n = (n / 10);
  }
  return lar;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    read(a, b);
    var x: dynamic = 0;
    var y: dynamic = 0;
    {
      var i: dynamic = (b.size() - 1);
      while ((i >= 0))
      {
        if ((b[i] == cpp_char("1")))
        {
          y = ((b.size() - 1) - i);
          break;
        }
        i -= 1;
      }
    }
    var flag: dynamic = 0;
    {
      var i: dynamic = (a.size() - 1);
      while ((i >= 0))
      {
        if ((a[i] == cpp_char("1")))
        {
          x = ((a.size() - 1) - i);
          if ((x >= y))
          {
            write((x - y), "\n");
            flag = 1;
            break;
          }
        }
        i -= 1;
      }
    }
    if ((flag == 0))
    {
      write(0, "\n");
    }
  }
}
