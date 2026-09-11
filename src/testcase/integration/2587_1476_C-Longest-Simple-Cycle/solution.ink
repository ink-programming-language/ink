// Translated from solution.cpp.

var ll: dynamic = dynamic;

func main() -> dynamic
{
  var int_cpp: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var int_cpp: dynamic = cpp_uninitialized();
    read(n);
    {
      var int_cpp: dynamic = 0;
      while ((i < n))
      {
        read(siz[i]);
        i += 1;
      }
    }
    {
      var int_cpp: dynamic = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    {
      var int_cpp: dynamic = 0;
      while ((i < n))
      {
        read(b[i]);
        b[i] = abs((b[i] - a[i]));
        i += 1;
      }
    }
    var dp: dynamic = cpp_construct((n + 1), 0);
    var ans: dynamic = -1e18;
    var temp: dynamic = 0;
    {
      var int_cpp: dynamic = 1;
      while ((i < n))
      {
        if ((b[i] == 0))
        {
          temp = 0;
        } else
        {
          temp = max((temp - b[i]), b[i]);
        }
        temp += ((2 + siz[i]) - 1);
        ans = max(ans, temp);
        i += 1;
      }
    }
    write(ans, cpp_char("\n"));
  }
  return 0;
}
