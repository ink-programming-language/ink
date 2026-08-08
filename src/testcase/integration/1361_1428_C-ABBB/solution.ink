// Translated from solution.cpp.

var MAXN = (2e5 + 5);

var str = cpp_array(MAXN);

var n: dynamic;

var t: dynamic;

func main()
{
  scanf("%d", (&t));
  while (cpp_update(t, "--"))
  {
    scanf("%d", (&n));
    scanf("%s", str);
    var n = strlen(str);
    var cur = 0;
    {
      var i = 0;
      while ((i < n))
      {
        if ((str[i] == cpp_char("B")))
        {
          if ((cur > 0))
          {
            cur -= 1;
          } else
          {
            cur += 1;
          }
        } else
        {
          cur += 1;
        }
        i += 1;
      }
    }
    printf("%d\n", cur);
  }
  return 0;
}
