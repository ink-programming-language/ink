// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var num = cpp_array(10);
  var i: dynamic;
  var j: dynamic;
  var flag: dynamic;
  var c: dynamic;
  scanf("%d", (&n));
  memset(num, 0, cpp_sizeof((num)));
  {
    i = 0;
    while ((i < 4))
    {
      getchar();
      {
        j = 0;
        while ((j < 4))
        {
          scanf("%c", (&c));
          if (isdigit(c))
          {
            num[(c - 48)] += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 1;
    flag = true;
    while ((i < 10))
    {
      if (((n * 2) < num[i]))
      {
        printf("NO\n");
        flag = false;
        break;
      }
      i += 1;
    }
  }
  if (flag)
  {
    printf("YES\n");
  }
}
