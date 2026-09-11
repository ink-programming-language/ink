// Translated from solution.cpp.

func main() -> dynamic
{
  var s: dynamic = [0];
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  while ((scanf("%s", s) != EOF))
  {
    n = strlen(s);
    m = 0;
    {
      var i: dynamic = 0;
      while ((i < (n / 2)))
      {
        if ((s[i] != s[((n - i) - 1)]))
        {
          m += 1;
        }
        i += 1;
      }
    }
    if ((m == 1))
    {
      puts("YES");
    } else
    {
      if ((((n % 2) == 1) && (m == 0)))
      {
        puts("YES");
      } else
      {
        puts("NO");
      }
    }
  }
  return 0;
}
