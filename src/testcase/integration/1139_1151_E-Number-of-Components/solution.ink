// Translated from solution.cpp.

var maxn = (1e5 + 10);

var str = cpp_array(maxn);

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(str[i]);
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((i == 0))
      {
        ans += ((1 * str[i]) * (((n - str[i]) + 1)));
      } else
      {
        if ((str[i] > str[(i - 1)]))
        {
          ans += ((1 * ((str[i] - str[(i - 1)]))) * (((n - str[i]) + 1)));
        } else
        {
          ans += ((1 * ((str[(i - 1)] - str[i]))) * str[i]);
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
