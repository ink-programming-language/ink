// Translated from solution.cpp.

var s = cpp_array(100005);

var ans = cpp_array(100005);

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  var flag = 0;
  var flag2 = 0;
  var cnt = 0;
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%s", s);
      if ((s[0] == cpp_char("0")))
      {
        flag = 1;
      } else
      {
        if ((s[0] != cpp_char("1")))
        {
          flag2 = 1;
          strcpy(ans, s);
        } else
        {
          var flag1 = 0;
          {
            var j = 1;
            while ((j < strlen(s)))
            {
              if ((s[j] != cpp_char("0")))
              {
                flag1 = 1;
                break;
              }
              j += 1;
            }
          }
          if ((flag1 == 1))
          {
            flag2 = 1;
            strcpy(ans, s);
          } else
          {
            cnt += (strlen(s) - 1);
          }
        }
      }
      i += 1;
    }
  }
  if ((flag == 1))
  {
    printf("0\n");
  } else
  {
    if ((flag2 == 1))
    {
      printf("%s", ans);
    } else
    {
      printf("1");
    }
    {
      var i = 0;
      while ((i < cnt))
      {
        printf("0");
        i += 1;
      }
    }
    printf("\n");
  }
  return 0;
}
