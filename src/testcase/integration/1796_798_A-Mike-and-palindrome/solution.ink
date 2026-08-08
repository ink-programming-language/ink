// Translated from solution.cpp.

func main()
{
  var s = [0];
  var n: dynamic;
  var m: dynamic;
  while ((scanf("%s", s) != EOF))
  {
    n = strlen(s);
    m = 0;
    {
      var i = 0;
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
