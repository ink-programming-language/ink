// Translated from solution.cpp.

var a: dynamic;

var b: dynamic;

var temp: dynamic;

var ans: dynamic;

func main()
{
  read(a, b);
  temp = abs((b - a));
  if (((temp % 2) == 0))
  {
    {
      var i = 1;
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
      var i = 1;
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
