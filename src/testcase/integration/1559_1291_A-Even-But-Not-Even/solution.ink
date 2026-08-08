// Translated from solution.cpp.

var T: dynamic;

var n: dynamic;

var num: dynamic;

var flag: dynamic;

var ans: dynamic;

var str: dynamic;

func checkc(c: dynamic)
{
  return ((((((c == cpp_char("1")) || (c == cpp_char("3"))) || (c == cpp_char("5"))) || (c == cpp_char("7"))) || (c == cpp_char("9"))));
}

func main()
{
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    scanf("%d", (&n));
    read(str);
    num = 0;
    flag = false;
    {
      var i = 0;
      while ((i < n))
      {
        if (checkc(str[i]))
        {
          num += 1;
          if ((num == 1))
          {
            ans = str[i];
          } else if ((num == 2))
          {
            write(ans, str[i], "\n");
            flag = true;
            break;
          }
        }
        i += 1;
      }
    }
    if ((!flag))
    {
      write("-1", "\n");
    }
  }
  return 0;
}
