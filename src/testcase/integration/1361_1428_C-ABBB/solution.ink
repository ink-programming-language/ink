// Translated from solution.cpp.

var MAXN: dynamic = (2e5 + 5);

var str: dynamic = cpp_array(MAXN);

var n: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d", (&t));
  while (cpp_update(t, "--"))
  {
    scanf("%d", (&n));
    scanf("%s", str);
    var n: dynamic = strlen(str);
    var cur: dynamic = 0;
    {
      var i: dynamic = 0;
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
