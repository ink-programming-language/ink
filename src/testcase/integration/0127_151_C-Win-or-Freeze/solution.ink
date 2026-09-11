// Translated from solution.cpp.

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
  while ((!isdigit(ch)))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (isdigit(ch))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (x * f);
}

func prim(x: dynamic) -> dynamic
{
  if (((x == 1) || (x == 4)))
  {
    return false;
  }
  if ((x <= 5))
  {
    return true;
  }
  var m: dynamic = cpp_cast(sqrt(x));
  if ((((x % 6) != 1) && ((x % 6) != 5)))
  {
    return false;
  }
  {
    var i: dynamic = 5;
    while ((i <= m))
    {
      if (((((x % i) == 0)) || (((x % ((i + 2))) == 0))))
      {
        return false;
      }
      i += 6;
    }
  }
  return true;
}

var pr: dynamic = cpp_array(10000000);

func prime(n: dynamic) -> dynamic
{
  pr[1] = true;
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      if ((!pr[i]))
      {
        {
          var j: dynamic = 2;
          while (((j * i) <= n))
          {
            pr[(i * j)] = true;
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var x: dynamic = cpp_uninitialized();
  read(x);
  if ((x == 1))
  {
    write(1, cpp_char("\n"), 0, cpp_char("\n"));
  } else if (prim(x))
  {
    write(1, cpp_char("\n"));
    write(0, cpp_char("\n"));
  } else
  {
    prime(cpp_cast(sqrt(x)));
    var res: dynamic = 0;
    var b: dynamic = -1;
    var up: dynamic = -1;
    {
      var i: dynamic = 2;
      while ((((i * 1) * i) <= x))
      {
        if (((!pr[i]) && (((x % ((i * 1))) == 0))))
        {
          res += 1;
          if ((up == -1))
          {
            up = i;
          } else if ((b == -1))
          {
            b = i;
          }
        }
        if ((res >= 3))
        {
          break;
        }
        i += 1;
      }
    }
    if ((res < 1))
    {
      write(2, cpp_char("\n"));
    } else if ((res == 1))
    {
      if ((((x % (((up * up) * 1))) == 0) && (((up * up)) != x)))
      {
        write(1, cpp_char("\n"));
        write(((up * 1) * up), cpp_char("\n"));
      } else
      {
        write(2, cpp_char("\n"));
      }
    } else if (((res == 2) && (((up * 1) * b) == x)))
    {
      write(2, cpp_char("\n"));
    } else
    {
      write(1, cpp_char("\n"));
      write(((up * 1) * b), cpp_char("\n"));
    }
  }
  return 0;
}
