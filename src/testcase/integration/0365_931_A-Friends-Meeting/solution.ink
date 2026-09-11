// Translated from solution.cpp.

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var temp: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(a, b);
  temp = abs((b - a));
  if (((temp % 2) == 0))
  {
    {
      var i: dynamic = 1;
      while ((i <= (temp / 2)))
      {
        ans += i;
        i += 1;
      }
    }
    write((ans * 2));
  } else
  {
    {
      var i: dynamic = 1;
      while ((i <= ((temp / 2) + 1)))
      {
        ans += i;
        i += 1;
      }
    }
    ans *= 2;
    ans -= ((temp / 2) + 1);
    write(ans);
  }
}
