// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var one: dynamic;

var two: dynamic;

var O: dynamic;

var s = cpp_array(3);

func main(argc: dynamic, argv: dynamic)
{
  scanf("%d%d", (&n), (&m));
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          scanf("%s", s);
          if ((strcmp(s, "11") == 0))
          {
            two += 1;
          } else if ((strcmp(s, "00") == 0))
          {
            O += 1;
          } else
          {
            one += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          if (j)
          {
            putchar(cpp_char(" "));
          }
          if (two)
          {
            printf("11");
            two -= 1;
          } else if ((((one + j) >= m) && ((i & 1))))
          {
            printf("01");
            one -= 1;
          } else if (((one + j) >= m))
          {
            printf("10");
            one -= 1;
          } else if (O)
          {
            printf("00");
            O -= 1;
          }
          j += 1;
        }
      }
      puts("");
      i += 1;
    }
  }
  return 0;
}
