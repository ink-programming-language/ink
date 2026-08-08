// Translated from solution.cpp.

var ll = dynamic;

func main()
{
  var int_cpp: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var int_cpp: dynamic;
    read(n);
    {
      var int_cpp = 0;
      while ((i < n))
      {
        read(siz[i]);
        i += 1;
      }
    }
    {
      var int_cpp = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    {
      var int_cpp = 0;
      while ((i < n))
      {
        read(b[i]);
        b[i] = abs((b[i] - a[i]));
        i += 1;
      }
    }
    var dp = cpp_construct((n + 1), 0);
    var ans = -1e18;
    var temp = 0;
    {
      var int_cpp = 1;
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
