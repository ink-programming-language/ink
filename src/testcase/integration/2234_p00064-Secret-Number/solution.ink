// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var sum: dynamic = 0;
  var s: dynamic = cpp_uninitialized();
  while ((cin >> s))
  {
    n = 0;
    {
      var i: dynamic = 0;
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
