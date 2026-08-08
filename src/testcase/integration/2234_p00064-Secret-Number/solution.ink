// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var sum = 0;
  var s: dynamic;
  while ((cin >> s))
  {
    n = 0;
    {
      var i = 0;
      while ((i < s.size()))
      {
        if (isdigit(s[i]))
        {
          n = (((10 * n) + s[i]) - cpp_char("0"));
        } else
        {
          sum += n;
          n = 0;
        }
        i += 1;
      }
    }
    sum += n;
  }
  write(sum, "\n");
  return 0;
}
