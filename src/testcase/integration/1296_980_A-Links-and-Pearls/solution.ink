// Translated from solution.cpp.

func main()
{
  var s = cpp_array(101);
  var l = 0;
  var p = 0;
  scanf("%s", s);
  {
    var i = 0;
    while ((i < strlen(s)))
    {
      if ((s[i] == cpp_char("o")))
      {
        p += 1;
      } else
      {
        l += 1;
      }
      i += 1;
    }
  }
  if ((p == 0))
  {
    printf("YES\n");
  } else if ((l % p))
  {
    printf("NO\n");
  } else
  {
    printf("Yes\n");
  }
  return 0;
}
